/*! @file
  @brief
  mruby bytecode executor.

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Fetch mruby VM bytecodes, decode and execute.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "vm_config.h"
#include <stddef.h>
#include <types.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

/***** Local headers ********************************************************/
#include "alloc.h"
#include "value.h"
#include "symbol.h"
#include "class.h"
#include "error.h"
#include "c_string.h"
#include "c_range.h"
#include "c_array.h"
#include "c_hash.h"
#include "global.h"
#include "load.h"
#include "console.h"
#include "opcode.h"
#include "vm.h"


/***** Constat values *******************************************************/
#define CALL_MAXARGS 15		// 15 is CALL_MAXARGS in mruby


/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
//! for getting the VM ID
static uint16_t free_vm_bitmap[MAX_VM_COUNT / 16 + 1];


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
//================================================================
/*! Method call by method name's id

  @param  vm		pointer to VM.
  @param  sym_id	method name symbol id
  @param  a		operand a
  @param  c		bit: 0-3=narg, 4-7=karg, 8=have block param flag.
  @retval 0  No error.
*/
static void send_by_name( struct VM *vm, mrbc_sym sym_id, int a, int c )
{
  // Does not support to keyword arguments.
  // thus, reorder arguments to mruby2 series compatible.
  int narg = c & 0x0f;
  int karg = (c >> 4) & 0x0f;
  mrbc_value *regs = vm->cur_regs;
  mrbc_value *recv = regs + a;

  // If it's packed in an array, expand it.
  if( narg == CALL_MAXARGS ) {
    mrbc_value argv = recv[1];
    narg = mrbc_array_size(&argv);
    int i;
    for( i = 0; i < narg; i++ ) {
      mrbc_incref( &argv.array->data[i] );
    }

    memmove( recv + narg + 1, recv + 2, sizeof(mrbc_value) * (karg * 2 + 1) );
    memcpy( recv + 1, argv.array->data, sizeof(mrbc_value) * narg );

    mrbc_decref(&argv);
  }

  // Convert keyword argument to hash.
  if( karg ) {
    mrbc_value h = mrbc_hash_new( vm, karg );
    if( !h.hash ) return;	// ENOMEM

    mrbc_value *r1 = recv + narg + 1;
    memcpy( h.hash->data, r1, sizeof(mrbc_value) * karg * 2 );
    h.hash->n_stored = karg * 2;

    mrbc_value block = r1[karg * 2];
    memset( r1 + 2, 0, sizeof(mrbc_value) * (karg * 2 - 1) );
    *r1++ = h;
    *r1 = block;
    narg++;
  }

  // is not have block
  if( (c >> 8) == 0 ) {
    mrbc_decref( recv + narg + 1 );
    mrbc_set_nil( recv + narg + 1 );
  }

  mrbc_class *cls = find_class_by_object(recv);
  mrbc_method method;
  if( mrbc_find_method( &method, cls, sym_id ) == 0 ) {
  // char buf[64];
  // sprintf(buf, "WTF - recv is %s   ...", mrbc_symid_to_str(cls->sym_id));
  // VDP_drawText(buf, 1, 1);

    mrbc_printf("Undefined local variable or method '%s' for %s\n",
		mrbc_symid_to_str(sym_id), mrbc_symid_to_str( cls->sym_id ));
    return;
  }

  if( method.c_func ) {
    // call C method.
    method.func(vm, recv, narg);
    if( sym_id == MRBC_SYM(call) ) return;
    if( sym_id == MRBC_SYM(new) ) return;

    int i;
    for( i = 1; i <= narg+1; i++ ) {
      mrbc_decref_empty( recv + i );
    }

  } else {
    // call Ruby method.
    mrbc_callinfo *callinfo = mrbc_push_callinfo(vm, sym_id, a, narg);
    callinfo->own_class = method.cls;

    vm->cur_irep = method.irep;
    vm->inst = vm->cur_irep->inst;
    vm->cur_regs = recv;
  }
}


//================================================================
/*! Find ensure catch handler
*/
static const mrbc_irep_catch_handler *find_catch_handler_ensure( const struct VM *vm )
{
  const mrbc_irep *irep = vm->cur_irep;
  int cnt = irep->clen;
  if( cnt == 0 ) return NULL;

  const mrbc_irep_catch_handler *catch_table =
    (const mrbc_irep_catch_handler *)(irep->inst + irep->ilen);
  uint32_t inst = vm->inst - irep->inst;

  for( cnt--; cnt >= 0 ; cnt-- ) {
    const mrbc_irep_catch_handler *handler = catch_table + cnt;
    // Catch type and range check
    if( (handler->type == 1) &&		// 1=CATCH_FILTER_ENSURE
	(bin_to_uint32(handler->begin) < inst) &&
	(inst <= bin_to_uint32(handler->end)) ) {
      return handler;
    }
  }

  return NULL;
}


//================================================================
/*! get the self object
*/
static mrbc_value * mrbc_get_self( struct VM *vm, mrbc_value *regs )
{
  mrbc_value *self = &regs[0];
  if( mrbc_type(*self) == MRBC_TT_PROC ) {
    mrbc_callinfo *callinfo = regs[0].proc->callinfo_self;
    if( callinfo ) {
      self = callinfo->cur_regs + callinfo->reg_offset;
    } else {
      self = &vm->regs[0];
    }
    assert( self->tt != MRBC_TT_PROC );
  }

  return self;
}


/***** Global functions *****************************************************/

//================================================================
/*! cleanup
*/
void mrbc_cleanup_vm(void)
{
  memset(free_vm_bitmap, 0, sizeof(free_vm_bitmap));
}


//================================================================
/*! get callee symbol id

  @param  vm	Pointer to VM
  @return	string
*/
mrbc_sym mrbc_get_callee_symid( struct VM *vm )
{
  uint8_t rb = vm->inst[-2];
  /* NOTE
     -2 is not always better value.
     This value is OP_SEND operator's B register.
  */
  return mrbc_irep_symbol_id(vm->cur_irep, rb);
}


//================================================================
/*! get callee name

  @param  vm	Pointer to VM
  @return	string
*/
const char *mrbc_get_callee_name( struct VM *vm )
{
  uint8_t rb = vm->inst[-2];
  return mrbc_irep_symbol_cstr(vm->cur_irep, rb);
}


//================================================================
/*! Push current status to callinfo stack
*/
mrbc_callinfo * mrbc_push_callinfo( struct VM *vm, mrbc_sym method_id, int reg_offset, int n_args )
{
  mrbc_callinfo *callinfo = mrbc_alloc(vm, sizeof(mrbc_callinfo));
  if( !callinfo ) return callinfo;

  callinfo->cur_irep = vm->cur_irep;
  callinfo->inst = vm->inst;
  callinfo->cur_regs = vm->cur_regs;
  callinfo->target_class = vm->target_class;

  callinfo->own_class = 0;
  callinfo->method_id = method_id;
  callinfo->reg_offset = reg_offset;
  callinfo->n_args = n_args;
  callinfo->is_called_super = 0;

  callinfo->prev = vm->callinfo_tail;
  vm->callinfo_tail = callinfo;

  return callinfo;
}


//================================================================
/*! Pop current status from callinfo stack
*/
void mrbc_pop_callinfo( struct VM *vm )
{
  assert( vm->callinfo_tail );

  // clear used register.
  mrbc_callinfo *callinfo = vm->callinfo_tail;
  mrbc_value *reg1 = vm->cur_regs + callinfo->cur_irep->nregs - callinfo->reg_offset;
  mrbc_value *reg2 = vm->cur_regs + vm->cur_irep->nregs;
  while( reg1 < reg2 ) {
    mrbc_decref_empty( reg1++ );
  }

  // copy callinfo to vm
  vm->cur_irep = callinfo->cur_irep;
  vm->inst = callinfo->inst;
  vm->cur_regs = callinfo->cur_regs;
  vm->target_class = callinfo->target_class;
  vm->callinfo_tail = callinfo->prev;

  mrbc_free(vm, callinfo);
}


//================================================================
/*! Open the VM.

  @param vm_arg	Pointer to mrbc_vm or NULL.
  @return	Pointer to mrbc_vm.
  @retval NULL	error.
*/
mrbc_vm *mrbc_vm_open( struct VM *vm_arg )
{
  mrbc_vm *vm = vm_arg;

  if( vm == NULL ) {
    // allocate memory.
    vm = mrbc_raw_alloc( sizeof(mrbc_vm) );
    if( vm == NULL ) return NULL;
  }

  // allocate vm id.
  int vm_id;
  for( vm_id = 0; vm_id < MAX_VM_COUNT; vm_id++ ) {
    int idx = vm_id >> 4;
    int bit = 1 << (vm_id & 0x0f);
    if( (free_vm_bitmap[idx] & bit) == 0 ) {
      free_vm_bitmap[idx] |= bit;		// found
      break;
    }
  }

  if( vm_id == MAX_VM_COUNT ) {
    if( vm_arg == NULL ) mrbc_raw_free(vm);
    return NULL;
  }
  vm_id++;

  // initialize attributes.
  memset(vm, 0, sizeof(mrbc_vm));	// caution: assume NULL is zero.
#if defined(MRBC_DEBUG)
  memcpy(vm->type, "VM", 2);
#endif
  if( vm_arg == NULL ) vm->flag_need_memfree = 1;
  vm->vm_id = vm_id;
  vm->regs_size = MAX_REGS_SIZE;

  return vm;
}


//================================================================
/*! Close the VM.

  @param  vm  Pointer to VM
*/
void mrbc_vm_close( struct VM *vm )
{
  // free vm id.
  int idx = (vm->vm_id-1) >> 4;
  int bit = 1 << ((vm->vm_id-1) & 0x0f);
  free_vm_bitmap[idx] &= ~bit;

  // free irep and vm
  if( vm->top_irep ) mrbc_irep_free( vm->top_irep );
  if( vm->flag_need_memfree ) mrbc_raw_free(vm);
}


//================================================================
/*! VM initializer.

  @param  vm  Pointer to VM
*/
void mrbc_vm_begin( struct VM *vm )
{
  vm->cur_irep = vm->top_irep;
  vm->inst = vm->cur_irep->inst;
  vm->cur_regs = vm->regs;
  vm->target_class = mrbc_class_object;
  vm->callinfo_tail = NULL;
  vm->ret_blk = NULL;
  vm->exception = mrbc_nil_value();
  vm->flag_preemption = 0;
  vm->flag_stop = 0;

  // set self to reg[0], others nil
  vm->regs[0] = mrbc_instance_new(vm, mrbc_class_object, 0);
  if( vm->regs[0].instance == NULL ) return;	// ENOMEM
  int i;
  for( i = 1; i < vm->regs_size; i++ ) {
    vm->regs[i] = mrbc_nil_value();
  }
}


//================================================================
/*! VM finalizer.

  @param  vm  Pointer to VM
*/
void mrbc_vm_end( struct VM *vm )
{
  if( mrbc_israised(vm) ) {
    mrbc_printf("Exception : %s (%s)\n",
		mrbc_symid_to_str(vm->exception.exception->cls->sym_id),
		vm->exception.exception->message ?
		  (const char *)vm->exception.exception->message :
		  mrbc_symid_to_str(vm->exception.exception->cls->sym_id) );
    mrbc_decref(&vm->exception);
  }
  assert( vm->ret_blk == 0 );

  int n_used = 0;
  int i;
  for( i = 0; i < vm->regs_size; i++ ) {
    //mrbc_printf("vm->regs[%d].tt = %d\n", i, mrbc_type(vm->regs[i]));
    if( mrbc_type(vm->regs[i]) != MRBC_TT_NIL ) n_used = i;
    mrbc_decref_empty(&vm->regs[i]);
  }
  (void)n_used;	// avoid warning.
#if defined(MRBC_DEBUG_REGS)
  mrbc_printf("Finally number of registers used was %d in VM %d.\n",
	      n_used, vm->vm_id );
#endif

#if defined(MRBC_ALLOC_VMID)
  mrbc_global_clear_vm_id();
  mrbc_free_all(vm);
#endif
}


/***** opecode functions ****************************************************/
#if defined(MRBC_SUPPORT_OP_EXT)
#define EXT , int ext
#else
#define EXT
#endif
//================================================================
/*! OP_NOP

  No operation
*/
static inline void op_nop( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_Z();
}


//================================================================
/*! OP_MOVE

  R[a] = R[b]
*/
static inline void op_move( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_incref(&regs[b]);
  mrbc_decref(&regs[a]);
  regs[a] = regs[b];
}


//================================================================
/*! OP_LOADL

  R[a] = Pool[b]
*/
static inline void op_loadl( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);
  regs[a] = mrbc_irep_pool_value(vm, b);
}


//================================================================
/*! OP_LOADI

  R[a] = mrb_int(b)
*/
static inline void op_loadi( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);
  mrbc_set_integer(&regs[a], b);
}


//================================================================
/*! OP_LOADINEG

  R[a] = mrb_int(-b)
*/
static inline void op_loadineg( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);
  mrbc_set_integer(&regs[a], -b);
}


//================================================================
/*! OP_LOADI_n (n=-1,0,1..7)

  R[a] = mrb_int(n)
*/
static inline void op_loadi_n( mrbc_vm *vm, mrbc_value *regs EXT )
{
  // get n
  int opcode = vm->inst[-1];
  int n = opcode - OP_LOADI_0;

  FETCH_B();

  mrbc_decref(&regs[a]);
  mrbc_set_integer(&regs[a], n);
}


//================================================================
/*! OP_LOADI16

  R[a] = mrb_int(b)
*/
static inline void op_loadi16( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BS();

  mrbc_decref(&regs[a]);
  int16_t signed_b = (int16_t)b;
  mrbc_set_integer(&regs[a], signed_b);
}


//================================================================
/*! OP_LOADI32

  R[a] = mrb_int((b<<16)+c)
*/
static inline void op_loadi32( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BSS();

  mrbc_decref(&regs[a]);
  mrbc_set_integer(&regs[a], (((int32_t)b<<16)+c));
}


//================================================================
/*! OP_LOADSYM

  R[a] = Syms[b]
*/
static inline void op_loadsym( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);
  mrbc_set_symbol(&regs[a], mrbc_irep_symbol_id(vm->cur_irep, b));
}


//================================================================
/*! OP_LOADNIL

  R[a] = nil
*/
static inline void op_loadnil( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_decref(&regs[a]);
  mrbc_set_nil(&regs[a]);
}


//================================================================
/*! OP_LOADSELF

  R[a] = self
*/
static inline void op_loadself( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_decref(&regs[a]);
  regs[a] = *mrbc_get_self( vm, regs );
  mrbc_incref( &regs[a] );
}


//================================================================
/*! OP_LOADT

  R[a] = true
*/
static inline void op_loadt( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_decref(&regs[a]);
  mrbc_set_true(&regs[a]);
}


//================================================================
/*! OP_LOADF

  R[a] = false
*/
static inline void op_loadf( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_decref(&regs[a]);
  mrbc_set_false(&regs[a]);
}


//================================================================
/*! OP_GETGV

  R[a] = getglobal(Syms[b])
*/
static inline void op_getgv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);
  mrbc_value *v = mrbc_get_global( mrbc_irep_symbol_id(vm->cur_irep, b) );
  if( v == NULL ) {
    mrbc_set_nil(&regs[a]);
  } else {
    mrbc_incref(v);
    regs[a] = *v;
  }
}


//================================================================
/*! OP_SETGV

  setglobal(Syms[b], R[a])
*/
static inline void op_setgv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_incref(&regs[a]);
  mrbc_set_global( mrbc_irep_symbol_id(vm->cur_irep, b), &regs[a] );
}


//================================================================
/*! OP_GETIV

  R[a] = ivget(Syms[b])
*/
static inline void op_getiv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  const char *sym_name = mrbc_irep_symbol_cstr(vm->cur_irep, b);
  mrbc_sym sym_id = mrbc_str_to_symid(sym_name+1);   // skip '@'
  mrbc_value *self = mrbc_get_self( vm, regs );

  mrbc_decref(&regs[a]);
  regs[a] = mrbc_instance_getiv(self, sym_id);
}


//================================================================
/*! OP_SETIV

  ivset(Syms[b],R[a])
*/
static inline void op_setiv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  const char *sym_name = mrbc_irep_symbol_cstr(vm->cur_irep, b);
  mrbc_sym sym_id = mrbc_str_to_symid(sym_name+1);   // skip '@'
  mrbc_value *self = mrbc_get_self( vm, regs );

  mrbc_instance_setiv(self, sym_id, &regs[a]);
}


//================================================================
/*! OP_GETCONST

  R[a] = constget(Syms[b])
*/
static inline void op_getconst( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_sym sym_id = mrbc_irep_symbol_id(vm->cur_irep, b);
  mrbc_class *cls = NULL;
  mrbc_value *v;

#define _GET_CLASS_CONST \
    v = mrbc_get_class_const(cls, sym_id); \
    if( v != NULL ) goto DONE;

  if( vm->callinfo_tail ) {
    // class environment
    if( vm->target_class != vm->callinfo_tail->target_class ) {
      // case 1: Normal class constant
      cls = vm->target_class;
      _GET_CLASS_CONST;

      // case 2: Nested class constant
      if( vm->callinfo_tail->target_class != mrbc_class_object ) {
        cls = vm->callinfo_tail->target_class;
        _GET_CLASS_CONST;
        mrbc_callinfo *tmp_ci = vm->callinfo_tail;
        while( tmp_ci->prev ) {
          cls = tmp_ci->prev->target_class;
          _GET_CLASS_CONST;
          tmp_ci = tmp_ci->prev;
        }
      }

      // case 3: Subclass constant
      // in order to be same with mruby's behavior (rather than CRuby)
      // We place it after `mrbc_get_const`
    }
  }

  if( vm->callinfo_tail ) cls = vm->callinfo_tail->own_class;
  while( cls != NULL ) {
    v = mrbc_get_class_const(cls, sym_id);
    if( v != NULL ) goto DONE;
    cls = cls->super;
  }

  v = mrbc_get_const(sym_id);
  if( v != NULL ) goto DONE;

  // case 3: Subclass constant
  if( vm->callinfo_tail ) {
    // class environment
    if( vm->target_class != vm->callinfo_tail->target_class ) {
      // To find super class
      cls = vm->target_class->super;
      while( cls != NULL ) {
        _GET_CLASS_CONST;
        cls = cls->super;
      }
    }
  }

#undef _GET_CLASS_CONST

  if( v == NULL ) {		// raise?
    mrbc_printf("NameError: uninitialized constant %s\n", mrbc_symid_to_str(sym_id));
    return;
  }

 DONE:
  mrbc_incref(v);
  mrbc_decref(&regs[a]);
  regs[a] = *v;
}


//================================================================
/*! OP_SETCONST

  constset(Syms[b],R[a])
*/
static inline void op_setconst( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_sym sym_id = mrbc_irep_symbol_id(vm->cur_irep, b);

  mrbc_incref(&regs[a]);
  if( mrbc_type(regs[0]) == MRBC_TT_CLASS ) {
    mrbc_set_class_const(regs[0].cls, sym_id, &regs[a]);
  } else {
    mrbc_set_const(sym_id, &regs[a]);
  }
}


//================================================================
/*! OP_GETMCNST

  R[a] = R[a]::Syms[b]
*/
static inline void op_getmcnst( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_sym sym_id = mrbc_irep_symbol_id(vm->cur_irep, b);
  mrbc_class *cls = regs[a].cls;
  mrbc_value *v;

  while( !(v = mrbc_get_class_const(cls, sym_id)) ) {
    cls = cls->super;
    if( !cls ) {	// raise?
      mrbc_printf("NameError: uninitialized constant %s::%s\n",
	mrbc_symid_to_str( regs[a].cls->sym_id ), mrbc_symid_to_str( sym_id ));
      return;
    }
  }

  mrbc_incref(v);
  mrbc_decref(&regs[a]);
  regs[a] = *v;
}


//================================================================
/*! OP_GETUPVAR

  R[a] = uvget(b,c)

  b: target offset of regs.
  c: nested block level.
*/
static inline void op_getupvar( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  assert( mrbc_type(regs[0]) == MRBC_TT_PROC );
  mrbc_callinfo *callinfo = regs[0].proc->callinfo;

  int i;
  for( i = 0; i < c; i++ ) {
    assert( callinfo );
    mrbc_value *reg0 = callinfo->cur_regs + callinfo->reg_offset;

    if( mrbc_type(*reg0) != MRBC_TT_PROC ) break;	// What to do?
    callinfo = reg0->proc->callinfo;
  }

  mrbc_value *p_val;
  if( callinfo == 0 ) {
    p_val = vm->regs + b;
  } else {
    p_val = callinfo->cur_regs + callinfo->reg_offset + b;
  }
  mrbc_incref( p_val );

  mrbc_decref( &regs[a] );
  regs[a] = *p_val;
}


//================================================================
/*! OP_SETUPVAR

  uvset(b,c,R[a])
*/
static inline void op_setupvar( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  assert( regs[0].tt == MRBC_TT_PROC );
  mrbc_callinfo *callinfo = regs[0].proc->callinfo;

  int i;
  for( i = 0; i < c; i++ ) {
    assert( callinfo );
    mrbc_value *reg0 = callinfo->cur_regs + callinfo->reg_offset;
    assert( reg0->tt == MRBC_TT_PROC );
    callinfo = reg0->proc->callinfo;
  }

  mrbc_value *p_val;
  if( callinfo == 0 ) {
    p_val = vm->regs + b;
  } else {
    p_val = callinfo->cur_regs + callinfo->reg_offset + b;
  }
  mrbc_decref( p_val );

  mrbc_incref( &regs[a] );
  *p_val = regs[a];
}


//================================================================
/*! OP_GETIDX

  R[a] = R[a][R[a+1]]
*/
static inline void op_getidx( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  send_by_name( vm, MRBC_SYMID_BL_BR, a, 1 );
}


//================================================================
/*! OP_SETIDX

  R[a][R[a+1]] = R[a+2]
*/
static inline void op_setidx( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  send_by_name( vm, MRBC_SYMID_BL_BR_EQ, a, 2 );
}


//================================================================
/*! OP_JMP

  pc+=a
*/
static inline void op_jmp( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_S();

  vm->inst += (int16_t)a;
}


//================================================================
/*! OP_JMPIF

  if R[a] pc+=b
*/
static inline void op_jmpif( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BS();

  if( regs[a].tt > MRBC_TT_FALSE ) {
    vm->inst += (int16_t)b;
  }
}


//================================================================
/*! OP_JMPNOT

  if !R[a] pc+=b
*/
static inline void op_jmpnot( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BS();

  if( regs[a].tt <= MRBC_TT_FALSE ) {
    vm->inst += (int16_t)b;
  }
}


//================================================================
/*! OP_JMPNIL

  if R[a]==nil pc+=b
*/
static inline void op_jmpnil( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BS();

  if( regs[a].tt == MRBC_TT_NIL ) {
    vm->inst += (int16_t)b;
  }
}


//================================================================
/*! OP_JMPUW

  unwind_and_jump_to(a)
*/
static inline void op_jmpuw( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_S();

  const uint8_t *jump_inst = vm->inst + (int16_t)a;

  // check catch handler (ensure)
  const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
  if( !handler ) {
    vm->inst = jump_inst;
    return;
  }

  // check whether the jump point is inside or outside the catch handler.
  uint32_t jump_point = jump_inst - vm->cur_irep->inst;
  if( (bin_to_uint32(handler->begin) < jump_point) &&
      (jump_point <= bin_to_uint32(handler->end)) ) {
    vm->inst = jump_inst;
    return;
  }

  // jump point is outside, thus jump to ensure.
  assert( vm->exception.tt == MRBC_TT_NIL );
  vm->exception.tt = MRBC_TT_JMPUW;
  vm->exception.handle = (void*)jump_inst;
  vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
}


//================================================================
/*! OP_EXCEPT

  R[a] = exc
*/
static inline void op_except( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_decref( &regs[a] );
  regs[a] = vm->exception;
  mrbc_set_nil( &vm->exception );
}


//================================================================
/*! OP_RESCUE

  R[b] = R[a].isa?(R[b])
*/
static inline void op_rescue( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  assert( regs[a].tt == MRBC_TT_EXCEPTION );
  assert( regs[b].tt == MRBC_TT_CLASS );

  int res = mrbc_obj_is_kind_of( &regs[a], regs[b].cls );
  mrbc_set_bool( &regs[b], res );
}


//================================================================
/*! OP_RAISEIF

  raise(R[a]) if R[a]
*/
static inline void op_raiseif( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  // save the parameter.
  mrbc_value ra = regs[a];
  regs[a].tt = MRBC_TT_EMPTY;

  switch( mrbc_type(ra) ) {
  case MRBC_TT_RETURN:		goto CASE_OP_RETURN;
  case MRBC_TT_RETURN_BLK:	goto CASE_OP_RETURN_BLK;
  case MRBC_TT_BREAK:		goto CASE_OP_BREAK;
  case MRBC_TT_JMPUW:		goto CASE_OP_JMPUW;
  case MRBC_TT_EXCEPTION:	goto CASE_OP_EXCEPTION;
  default: break;
  }

  assert( mrbc_type(ra) == MRBC_TT_NIL );
  assert( mrbc_type(vm->exception) == MRBC_TT_NIL );
  return;


CASE_OP_RETURN:
{
  // find ensure that still needs to be executed.
  const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
  if( handler ) {
    vm->exception = ra;
    vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
    return;
  }

  // set the return value and return to caller.
  mrbc_decref(&regs[0]);
  regs[0] = regs[ vm->cur_irep->nregs ];
  regs[ vm->cur_irep->nregs ].tt = MRBC_TT_EMPTY;

  mrbc_pop_callinfo(vm);
  return;
}


CASE_OP_RETURN_BLK:
{
  assert( vm->ret_blk );

  // return to the proc generated level.
  while( 1 ) {
    // find ensure that still needs to be executed.
    const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
    if( handler ) {
      vm->exception = ra;
      vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
      return;
    }

    // Is it the origin (generator) of proc?
    if( vm->callinfo_tail == vm->ret_blk->callinfo_self ) break;

    mrbc_pop_callinfo(vm);
  }

  // top level return ?
  if( vm->callinfo_tail == NULL ) {
    mrbc_decref(&(mrbc_value){.tt = MRBC_TT_PROC, .proc = vm->ret_blk});
    vm->ret_blk = 0;

    vm->flag_preemption = 1;
    vm->flag_stop = 1;
    return;
  }

  // set the return value and return to caller.
  mrbc_value *reg0 = vm->callinfo_tail->cur_regs + vm->callinfo_tail->reg_offset;
  mrbc_decref(reg0);
  *reg0 = vm->ret_blk->ret_val;

  mrbc_decref(&(mrbc_value){.tt = MRBC_TT_PROC, .proc = vm->ret_blk});
  vm->ret_blk = 0;

  mrbc_pop_callinfo(vm);
  return;
}


CASE_OP_BREAK: {
  assert( vm->ret_blk );

  // return to the proc generated level.
  int reg_offset = 0;
  while( vm->callinfo_tail != vm->ret_blk->callinfo_self ) {
    // find ensure that still needs to be executed.
    const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
    if( handler ) {
      vm->exception = ra;
      vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
      return;
    }

    reg_offset = vm->callinfo_tail->reg_offset;
    mrbc_pop_callinfo(vm);
  }

  // set the return value.
  mrbc_value *reg0 = vm->cur_regs + reg_offset;
  mrbc_decref(reg0);
  *reg0 = vm->ret_blk->ret_val;

  mrbc_decref(&(mrbc_value){.tt = MRBC_TT_PROC, .proc = vm->ret_blk});
  vm->ret_blk = 0;
  return;
}


CASE_OP_JMPUW:
{
  // find ensure that still needs to be executed.
  const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
  if( !handler ) {
    vm->inst = ra.handle;
    return;
  }

  // check whether the jump point is inside or outside the catch handler.
  uint32_t jump_point = (uint8_t *)ra.handle - vm->cur_irep->inst;
  if( (bin_to_uint32(handler->begin) < jump_point) &&
      (jump_point <= bin_to_uint32(handler->end)) ) {
    vm->inst = ra.handle;
    return;
  }

  // jump point is outside, thus jump to ensure.
  assert( vm->exception.tt == MRBC_TT_NIL );
  vm->exception = ra;
  vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
  return;
}


CASE_OP_EXCEPTION:
{
  vm->exception = ra;
  vm->flag_preemption = 2;
  return;
}
}


//================================================================
/*! OP_SSEND

  R[a] = self.send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..) (c=n|k<<4)
*/
static inline void op_ssend( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  mrbc_decref( &regs[a] );
  regs[a] = *mrbc_get_self( vm, regs );
  mrbc_incref( &regs[a] );

  send_by_name( vm, mrbc_irep_symbol_id(vm->cur_irep, b), a, c );
}



//================================================================
/*! OP_SSENDB

  R[a] = self.send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..,&R[a+n+2k+1])
*/
static inline void op_ssendb( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  mrbc_decref( &regs[a] );
  regs[a] = *mrbc_get_self( vm, regs );
  mrbc_incref( &regs[a] );

  send_by_name( vm, mrbc_irep_symbol_id(vm->cur_irep, b), a, c | 0x100 );
}



//================================================================
/*! OP_SEND

  R[a] = R[a].send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..) (c=n|k<<4)
*/
static inline void op_send( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  send_by_name( vm, mrbc_irep_symbol_id(vm->cur_irep, b), a, c );
}


//================================================================
/*! OP_SENDB

  R[a] = R[a].send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..,&R[a+n+2k+1])
*/
static inline void op_sendb( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  send_by_name( vm, mrbc_irep_symbol_id(vm->cur_irep, b), a, c | 0x100 );
}


//================================================================
/*! OP_SUPER

  R[a] = super(R[a+1],... ,R[a+b+1])
*/
static inline void op_super( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  // set self to new regs[0]
  mrbc_value *recv = mrbc_get_self(vm, regs);
  assert( recv->tt != MRBC_TT_PROC );

  mrbc_incref( recv );
  mrbc_decref( &regs[a] );
  regs[a] = *recv;

  if( (b & 0x0f) == CALL_MAXARGS ) {
    /* (note)
       on mrbc ver 3.1
         b = 15  in initialize method.
	 b = 255 in other method.
    */

    // expand array
    assert( regs[a+1].tt == MRBC_TT_ARRAY );

    mrbc_value argary = regs[a+1];
    regs[a+1].tt = MRBC_TT_EMPTY;
    mrbc_value proc = regs[a+2];
    regs[a+2].tt = MRBC_TT_EMPTY;

    int argc = mrbc_array_size(&argary);
    int i, j;
    for( i = 0, j = a+1; i < argc; i++, j++ ) {
      mrbc_decref( &regs[j] );
      regs[j] = argary.array->data[i];
    }
    mrbc_array_delete_handle(&argary);

    regs[j] = proc;
    b = argc;
  }

  // find super class
  mrbc_callinfo *callinfo = vm->callinfo_tail;
  mrbc_class *cls = callinfo->own_class;
  mrbc_method method;

  assert( cls );
  cls = cls->super;
  assert( cls );
  if( mrbc_find_method( &method, cls, callinfo->method_id ) == 0 ) {
    mrbc_printf("Undefined method '%s' for %s\n",
	mrbc_symid_to_str(callinfo->method_id), mrbc_symid_to_str(cls->sym_id));
    return;	// raise?
  }

  if( method.c_func ) {
    mrbc_printf("Not supported!\n");	// TODO
    return;	// raise?
  }

  callinfo = mrbc_push_callinfo(vm, callinfo->method_id, a, b);
  callinfo->own_class = method.cls;
  callinfo->is_called_super = 1;

  vm->cur_irep = method.irep;
  vm->inst = vm->cur_irep->inst;
  vm->cur_regs += a;
}


//================================================================
/*! OP_ARGARY

  R[a] = argument array (16=m5:r1:m5:d1:lv4)

  flags: mmmm_mrmm_mmmd_llll
*/
static inline void op_argary( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BS();

  int m1 = (b >> 11) & 0x3f;
  int d  = (b >>  4) & 0x01;
  int lv = b & 0x0f;

  if( b & 0x400 ) {	// check REST parameter.
    // TODO: want to support.
    mrbc_printf("Not support rest parameter by super.\n");
    return;		// raise?
  }
  if( b & 0x3e0 ) {	// check m2 parameter.
    mrbc_printf("ArgumentError: not support m2 or keyword argument.\n");
    return;		// raise?
  }

  mrbc_value *reg0 = regs;

  // rewind proc nest
  if( lv ) {
    assert( mrbc_type(*reg0) == MRBC_TT_PROC );
    mrbc_callinfo *callinfo = reg0->proc->callinfo;
    assert( callinfo );

    int i;
    for( i = 1; i < lv; i ++ ) {
      reg0 = callinfo->cur_regs + callinfo->reg_offset;
      assert( mrbc_type(*reg0) == MRBC_TT_PROC );
      callinfo = reg0->proc->callinfo;
      assert( callinfo );
    }

    reg0 = callinfo->cur_regs + callinfo->reg_offset;
  }

  // create arguent array.
  int array_size = m1 + d;
  mrbc_value val = mrbc_array_new( vm, array_size );
  if( !val.array ) return;	// ENOMEM raise?

  int i;
  for( i = 0; i < array_size; i++ ) {
    mrbc_array_push( &val, &reg0[i+1] );
    mrbc_incref( &reg0[i+1] );
  }

  mrbc_decref( &regs[a] );
  regs[a] = val;

  // copy a block object
  mrbc_decref( &regs[a+1] );
  regs[a+1] = reg0[array_size+1];
  mrbc_incref( &regs[a+1] );
}


//================================================================
/*! OP_ENTER

  arg setup according to flags (23=m5:o5:r1:m5:k5:d1:b1)

  flags: 0mmm_mmoo_ooor_mmmm_mkkk_kkdb
*/
static inline void op_enter( mrbc_vm *vm, mrbc_value *regs EXT )
{
#define FLAG_REST	0x1000
#define FLAG_M2		0x0f80
#define FLAG_KW		0x007c
#define FLAG_DICT	0x0002
#define FLAG_BLOCK	0x0001

  FETCH_W();

  // Check the number of registers to use.
  int reg_use_max = regs - vm->regs + vm->cur_irep->nregs;
  if( reg_use_max >= vm->regs_size ) {
    mrbc_raise(vm, MRBC_CLASS(Exception), "MAX_REGS_SIZE overflow.");
    return;
  }

  int m1 = (a >> 18) & 0x1f;	// # of required parameters 1
  int o  = (a >> 13) & 0x1f;	// # of optional parameters
  int argc = vm->callinfo_tail->n_args;

  if( a & (FLAG_M2|FLAG_KW) ) {	// check m2 and k parameter.
    mrbc_printf("ArgumentError: not support m2 or keyword argument.\n");
    return;		// raise?
  }

  if( argc < m1 && mrbc_type(regs[0]) != MRBC_TT_PROC ) {
    mrbc_printf("ArgumentError: wrong number of arguments.\n");
    return;		// raise?
  }

  // save proc (or nil) object.
  mrbc_value proc = regs[argc+1];
  regs[argc+1].tt = MRBC_TT_EMPTY;

  // support yield [...] pattern, to expand array.
  if( mrbc_type(regs[0]) == MRBC_TT_PROC &&
      mrbc_type(regs[1]) == MRBC_TT_ARRAY &&
      argc == 1 && m1 > 1 ) {
    mrbc_value argary = regs[1];
    regs[1].tt = MRBC_TT_EMPTY;

    argc = mrbc_array_size(&argary);
    if( argc < m1 ) argc = m1;

    int i;
    for( i = 0; i < argc; i++ ) {
      if( mrbc_array_size(&argary) > i ) {
	regs[i+1] = argary.array->data[i];
      } else {
	mrbc_set_nil( &regs[i+1] );
      }
    }
    mrbc_array_delete_handle( &argary );
  }

  // dictionary or rest parameter exists.
  if( a & (FLAG_DICT|FLAG_REST) ) {
    mrbc_value dict;
    if( a & FLAG_DICT ) {
      if( (argc - m1) > 0 && mrbc_type(regs[argc]) == MRBC_TT_HASH ) {
	dict = regs[argc];
	regs[argc--].tt = MRBC_TT_EMPTY;
      } else {
	dict = mrbc_hash_new( vm, 0 );
      }
    }

    mrbc_value rest;
    if( a & FLAG_REST ) {
      int rest_size = argc - m1 - o;
      if( rest_size < 0 ) rest_size = 0;
      rest = mrbc_array_new(vm, rest_size);
      if( !rest.array ) return;	// ENOMEM raise?

      int rest_reg = m1 + o + 1;
      int i;
      for( i = 0; i < rest_size; i++ ) {
	mrbc_array_push( &rest, &regs[rest_reg] );
	regs[rest_reg++].tt = MRBC_TT_EMPTY;
      }
    }

    // reorder arguments.
    int i;
    for( i = argc; i < m1; ) {
      mrbc_set_nil( &regs[++i] );
    }
    i = m1 + o;
    if( a & FLAG_REST ) {
      regs[++i] = rest;
    }
    if( a & FLAG_DICT ) {
      regs[++i] = dict;
    }
    regs[i+1] = proc;
    vm->callinfo_tail->n_args = i;

  } else {
    // reorder arguments.
    int i;
    for( i = argc; i < m1; ) {
      mrbc_set_nil( &regs[++i] );
    }
    i = m1 + o;
    regs[i+1] = proc;
    vm->callinfo_tail->n_args = i;
  }

  // prepare for get default arguments.
  int jmp_ofs = argc - m1;
  if( jmp_ofs > 0 ) {
    if( jmp_ofs > o ) {
      jmp_ofs = o;

      if( !(a & FLAG_REST) && mrbc_type(regs[0]) != MRBC_TT_PROC ) {
	mrbc_printf("ArgumentError: wrong number of arguments.\n");
	return;		// raise?
      }
    }
    vm->inst += jmp_ofs * 3;	// 3 = bytecode size of OP_JMP
  }

#undef FLAG_REST
#undef FLAG_M2
#undef FLAG_KW
#undef FLAG_DICT
#undef FLAG_BLOCK
}


//================================================================
/*! op_return, op_return_blk subroutine.
*/
static inline void op_return__sub( mrbc_vm *vm, mrbc_value *regs, int a )
{
  // If have a ensure, jump to it.
  if( vm->cur_irep->clen ) {
    const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
    if( handler ) {
      assert( vm->exception.tt == MRBC_TT_NIL );

      // Save the return value in the last+1 register.
      regs[ vm->cur_irep->nregs ] = regs[a];
      regs[a].tt = MRBC_TT_EMPTY;

      vm->exception.tt = MRBC_TT_RETURN;
      vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
      return;
    }
  }

  // return without anything if top level.
  if( vm->callinfo_tail == NULL ) {
    if( vm->flag_permanence ) mrbc_incref(&regs[a]);
    vm->flag_preemption = 1;
    vm->flag_stop = 1;
    return;
  }

  // not in initialize method, set return value.
  if( vm->callinfo_tail->method_id != MRBC_SYM(initialize) ) goto SET_RETURN;

  // not called by op_super, ignore return value.
  if( !vm->callinfo_tail->is_called_super ) goto RETURN;

  // set the return value
 SET_RETURN:
  mrbc_decref(&regs[0]);
  regs[0] = regs[a];
  regs[a].tt = MRBC_TT_EMPTY;

 RETURN:
  mrbc_pop_callinfo(vm);
}


//================================================================
/*! OP_RETURN

  return R[a] (normal)
*/
static inline void op_return( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  op_return__sub( vm, regs, a );
}


//================================================================
/*! OP_RETURN_BLK

  return R[a] (in-block return)
*/
static inline void op_return_blk( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  if( mrbc_type(regs[0]) != MRBC_TT_PROC ) {
    op_return__sub( vm, regs, a );
    return;
  }

  // Save the return value in the proc object.
  mrbc_incref( &regs[0] );
  vm->ret_blk = regs[0].proc;
  vm->ret_blk->ret_val = regs[a];
  regs[a].tt = MRBC_TT_EMPTY;

  // return to the proc generated level.
  while( 1 ) {
    // If have a ensure, jump to it.
    const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
    if( handler ) {
      assert( vm->exception.tt == MRBC_TT_NIL );
      vm->exception.tt = MRBC_TT_RETURN_BLK;
      vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
      return;
    }

    // Is it the origin (generator) of proc?
    if( vm->callinfo_tail == vm->ret_blk->callinfo_self ) break;

    mrbc_pop_callinfo(vm);
  }

  // top level return ?
  if( vm->callinfo_tail == NULL ) {
    vm->flag_preemption = 1;
    vm->flag_stop = 1;
  } else {
    // set the return value.
    mrbc_decref(&vm->cur_regs[0]);
    vm->cur_regs[0] = vm->ret_blk->ret_val;

    mrbc_pop_callinfo(vm);
  }

  mrbc_decref(&(mrbc_value){.tt = MRBC_TT_PROC, .proc = vm->ret_blk});
  vm->ret_blk = 0;
}


//================================================================
/*! OP_BREAK

  break R[a]
*/
static inline void op_break( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  assert( regs[0].tt == MRBC_TT_PROC );

  // Save the return value in the proc object.
  mrbc_incref( &regs[0] );
  vm->ret_blk = regs[0].proc;
  vm->ret_blk->ret_val = regs[a];
  regs[a].tt = MRBC_TT_EMPTY;

  // return to the proc generated level.
  int reg_offset = 0;
  while( 1 ) {
    // If have a ensure, jump to it.
    const mrbc_irep_catch_handler *handler = find_catch_handler_ensure(vm);
    if( handler ) {
      assert( vm->exception.tt == MRBC_TT_NIL );
      vm->exception.tt = MRBC_TT_BREAK;
      vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
      return;
    }

    // Is it the origin (generator) of proc?
    if( vm->callinfo_tail == vm->ret_blk->callinfo_self ) break;

    reg_offset = vm->callinfo_tail->reg_offset;
    mrbc_pop_callinfo(vm);
  }

  // set the return value.
  mrbc_value *reg0 = vm->cur_regs + reg_offset;
  mrbc_decref(reg0);
  *reg0 = vm->ret_blk->ret_val;

  mrbc_decref(&(mrbc_value){.tt = MRBC_TT_PROC, .proc = vm->ret_blk});
  vm->ret_blk = 0;
}


//================================================================
/*! OP_BLKPUSH

  R[a] = block (16=m5:r1:m5:d1:lv4)
*/
static inline void op_blkpush( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BS();

  int m1 = (b >> 11) & 0x3f;
  int r  = (b >> 10) & 0x01;
  int m2 = (b >>  5) & 0x1f;
  int d  = (b >>  4) & 0x01;
  int lv = (b      ) & 0x0f;

  if( m2 ) {
    mrbc_printf("ArgumentError: not support m2 or keyword argument.\n");
    return;		// raise?
  }

  int offset = m1 + r + d + 1;
  mrbc_value *blk;

  if( lv == 0 ) {
    // current env
    blk = regs + offset;
  } else {
    // upper env
    assert( regs[0].tt == MRBC_TT_PROC );

    mrbc_callinfo *callinfo = regs[0].proc->callinfo_self;
    blk = callinfo->cur_regs + callinfo->reg_offset + offset;
  }
  if( blk->tt != MRBC_TT_PROC ) {
    mrbc_printf("no block given (yield) (LocalJumpError)\n");
    return;	// raise?
  }

  mrbc_incref(blk);
  mrbc_decref(&regs[a]);
  regs[a] = *blk;
}


//================================================================
/*! OP_ADD

  R[a] = R[a]+R[a+1]
*/
static inline void op_add( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  if( regs[a].tt == MRBC_TT_INTEGER ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Integer, Integer
      regs[a].i += regs[a+1].i;
      return;
    }
#if MRBC_USE_FLOAT
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Integer, Float
      regs[a].tt = MRBC_TT_FLOAT;
      regs[a].d = regs[a].i + regs[a+1].d;
      return;
    }
  }
  if( regs[a].tt == MRBC_TT_FLOAT ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Float, Integer
      regs[a].d += regs[a+1].i;
      return;
    }
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Float, Float
      regs[a].d += regs[a+1].d;
      return;
    }
#endif
  }

  // other case
  send_by_name( vm, MRBC_SYM(PLUS), a, 1 );
}


//================================================================
/*! OP_ADDI

  R[a] = R[a]+mrb_int(b)
*/
static inline void op_addi( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  if( regs[a].tt == MRBC_TT_INTEGER ) {
    regs[a].i += b;
    return;
  }

#if MRBC_USE_FLOAT
  if( regs[a].tt == MRBC_TT_FLOAT ) {
    regs[a].d += b;
    return;
  }
#endif

  mrbc_printf("Not supported!\n");
}


//================================================================
/*! OP_SUB

  R[a] = R[a]-R[a+1]
*/
static inline void op_sub( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  if( regs[a].tt == MRBC_TT_INTEGER ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Integer, Integer
      regs[a].i -= regs[a+1].i;
      return;
    }
#if MRBC_USE_FLOAT
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Integer, Float
      regs[a].tt = MRBC_TT_FLOAT;
      regs[a].d = regs[a].i - regs[a+1].d;
      return;
    }
  }
  if( regs[a].tt == MRBC_TT_FLOAT ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Float, Integer
      regs[a].d -= regs[a+1].i;
      return;
    }
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Float, Float
      regs[a].d -= regs[a+1].d;
      return;
    }
#endif
  }

  // other case
  send_by_name( vm, MRBC_SYM(MINUS), a, 1 );
}


//================================================================
/*! OP_SUBI

  R[a] = R[a]-mrb_int(b)
*/
static inline void op_subi( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  if( regs[a].tt == MRBC_TT_INTEGER ) {
    regs[a].i -= b;
    return;
  }

#if MRBC_USE_FLOAT
  if( regs[a].tt == MRBC_TT_FLOAT ) {
    regs[a].d -= b;
    return;
  }
#endif

  mrbc_printf("Not supported!\n");
}


//================================================================
/*! OP_MUL

  R[a] = R[a]*R[a+1]
*/
static inline void op_mul( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  if( regs[a].tt == MRBC_TT_INTEGER ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Integer, Integer
      regs[a].i *= regs[a+1].i;
      return;
    }
#if MRBC_USE_FLOAT
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Integer, Float
      regs[a].tt = MRBC_TT_FLOAT;
      regs[a].d = regs[a].i * regs[a+1].d;
      return;
    }
  }
  if( regs[a].tt == MRBC_TT_FLOAT ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Float, Integer
      regs[a].d *= regs[a+1].i;
      return;
    }
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Float, Float
      regs[a].d *= regs[a+1].d;
      return;
    }
#endif
  }

  // other case
  send_by_name( vm, MRBC_SYM(MUL), a, 1 );
}


//================================================================
/*! OP_DIV

  R[a] = R[a]/R[a+1]
*/
static inline void op_div( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  if( regs[a].tt == MRBC_TT_INTEGER ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Integer, Integer
      regs[a].i /= regs[a+1].i;
      return;
    }
#if MRBC_USE_FLOAT
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Integer, Float
      regs[a].tt = MRBC_TT_FLOAT;
      regs[a].d = regs[a].i / regs[a+1].d;
      return;
    }
  }
  if( regs[a].tt == MRBC_TT_FLOAT ) {
    if( regs[a+1].tt == MRBC_TT_INTEGER ) {     // in case of Float, Integer
      regs[a].d /= regs[a+1].i;
      return;
    }
    if( regs[a+1].tt == MRBC_TT_FLOAT ) {      // in case of Float, Float
      regs[a].d /= regs[a+1].d;
      return;
    }
#endif
  }

  // other case
  send_by_name( vm, MRBC_SYM(DIV), a, 1 );
}


//================================================================
/*! OP_EQ

  R[a] = R[a]==R[a+1]
*/
static inline void op_eq( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  // TODO: case OBJECT == OBJECT is not supported.
  int result = mrbc_compare(&regs[a], &regs[a+1]);

  mrbc_decref(&regs[a]);
  regs[a].tt = result ? MRBC_TT_FALSE : MRBC_TT_TRUE;
}


//================================================================
/*! OP_LT

  R[a] = R[a]<R[a+1]
*/
static inline void op_lt( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  // TODO: case OBJECT < OBJECT is not supported.
  int result = mrbc_compare(&regs[a], &regs[a+1]);

  mrbc_decref(&regs[a]);
  regs[a].tt = result < 0 ? MRBC_TT_TRUE : MRBC_TT_FALSE;
}


//================================================================
/*! OP_LE

  R[a] = R[a]<=R[a+1]
*/
static inline void op_le( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  // TODO: case OBJECT <= OBJECT is not supported.
  int result = mrbc_compare(&regs[a], &regs[a+1]);

  mrbc_decref(&regs[a]);
  regs[a].tt = result <= 0 ? MRBC_TT_TRUE : MRBC_TT_FALSE;
}


//================================================================
/*! OP_GT

  R[a] = R[a]>R[a+1]
*/
static inline void op_gt( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  // TODO: case OBJECT > OBJECT is not supported.
  int result = mrbc_compare(&regs[a], &regs[a+1]);

  mrbc_decref(&regs[a]);
  regs[a].tt = result > 0 ? MRBC_TT_TRUE : MRBC_TT_FALSE;
}


//================================================================
/*! OP_GE

  R[a] = R[a]>=R[a+1]
*/
static inline void op_ge( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  // TODO: case OBJECT >= OBJECT is not supported.
  int result = mrbc_compare(&regs[a], &regs[a+1]);

  mrbc_decref(&regs[a]);
  regs[a].tt = result >= 0 ? MRBC_TT_TRUE : MRBC_TT_FALSE;
}


//================================================================
/*! OP_ARRAY

  R[a] = ary_new(R[a],R[a+1]..R[a+b])
*/
static inline void op_array( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_value value = mrbc_array_new(vm, b);
  if( value.array == NULL ) return;  // ENOMEM

  memcpy( value.array->data, &regs[a], sizeof(mrbc_value) * b );
  memset( &regs[a], 0, sizeof(mrbc_value) * b );
  value.array->n_stored = b;

  mrbc_decref(&regs[a]);
  regs[a] = value;
}


//================================================================
/*! OP_ARRAY2

  R[a] = ary_new(R[b],R[b+1]..R[b+c])
*/
static inline void op_array2( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  mrbc_value value = mrbc_array_new(vm, c);
  if( value.array == NULL ) return;  // ENOMEM

  int i;
  for( i = 0; i < c; i++ ) {
    mrbc_incref( &regs[b+i] );
    value.array->data[i] = regs[b+i];
  }
  value.array->n_stored = c;

  mrbc_decref(&regs[a]);
  regs[a] = value;
}


//================================================================
/*! OP_ARYCAT

  ary_cat(R[a],R[a+1])
*/
static inline void op_arycat( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  if( regs[a].tt == MRBC_TT_NIL ) {
    // arycat(nil, [...]) #=> [...]
    assert( regs[a+1].tt == MRBC_TT_ARRAY );
    regs[a] = regs[a+1];
    regs[a+1].tt = MRBC_TT_NIL;

    return;
  }

  assert( regs[a  ].tt == MRBC_TT_ARRAY );
  assert( regs[a+1].tt == MRBC_TT_ARRAY );

  int size_1 = regs[a  ].array->n_stored;
  int size_2 = regs[a+1].array->n_stored;
  int new_size = size_1 + regs[a+1].array->n_stored;

  // need resize?
  if( regs[a].array->data_size < new_size ) {
    mrbc_array_resize(&regs[a], new_size);
  }

  int i;
  for( i = 0; i < size_2; i++ ) {
    mrbc_incref( &regs[a+1].array->data[i] );
    regs[a].array->data[size_1+i] = regs[a+1].array->data[i];
  }
  regs[a].array->n_stored = new_size;
}


//================================================================
/*! OP_ARYPUSH

  ary_push(R[a],R[a+1]..R[a+b])
*/
static inline void op_arypush( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  int sz1 = mrbc_array_size(&regs[a]);

  int ret = mrbc_array_resize(&regs[a], sz1 + b);
  if( ret != 0 ) return;	// ENOMEM ?

  // data copy.
  memcpy( regs[a].array->data + sz1, &regs[a+1], sizeof(mrbc_value) * b );
  memset( &regs[a+1], 0, sizeof(mrbc_value) * b );
  regs[a].array->n_stored = sz1 + b;
}


//================================================================
/*! OP_ARYDUP

  R[a] = ary_dup(R[a])
*/
static inline void op_arydup( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_value ret = mrbc_array_dup( vm, &regs[a] );
  mrbc_decref(&regs[a]);
  regs[a] = ret;
}


//================================================================
/*! OP_AREF

  R[a] = R[b][c]
*/
static inline void op_aref( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  mrbc_value *src = &regs[b];
  mrbc_value *dst = &regs[a];

  mrbc_decref( dst );

  if( mrbc_type(*src) == MRBC_TT_ARRAY ) {
    // src is Array
    *dst = mrbc_array_get(src, c);
    mrbc_incref(dst);
  } else {
    // src is not Array
    if( c == 0 ) {
      mrbc_incref(src);
      *dst = *src;
    } else {
      mrbc_set_nil( dst );
    }
  }
}


//================================================================
/*! OP_ASET

  R[b][c] = R[a]
*/
static inline void op_aset( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  assert( mrbc_type(regs[b]) == MRBC_TT_ARRAY );

  mrbc_incref( &regs[b] );
  mrbc_array_set(&regs[a], c, &regs[b]);
}


//================================================================
/*! OP_APOST

  *R[a],R[a+1]..R[a+c] = R[a][b..]
*/
static inline void op_apost( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BBB();

  mrbc_value src = regs[a];
  if( mrbc_type(src) != MRBC_TT_ARRAY ) {
    src = mrbc_array_new(vm, 1);
    src.array->data[0] = regs[a];
    src.array->n_stored = 1;
  }

  int pre  = b;
  int post = c;
  int len = mrbc_array_size(&src);

  if( len > pre + post ) {
    int ary_size = len - pre - post;
    regs[a] = mrbc_array_new(vm, ary_size);
    // copy elements
    int i;
    for( i = 0; i < ary_size; i++ ) {
      regs[a].array->data[i] = src.array->data[pre+i];
      mrbc_incref( &regs[a].array->data[i] );
    }
    regs[a].array->n_stored = ary_size;

  } else {
//    char buf[128];
 //   sprintf(buf, "else bloc, len: %d, pre: %d, post: %d", len, pre, post);
  //VDP_drawText(buf, 1, 17);
    assert(!"Not support this case in op_apost.");
 //  VDP_drawText("after assert", 10, 18);
    // empty
    regs[a] = mrbc_array_new(vm, 0);
  }

  mrbc_decref(&src);
}


//================================================================
/*! OP_INTERN

  R[a] = intern(R[a])
*/
static inline void op_intern( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  assert( regs[a].tt == MRBC_TT_STRING );

  mrbc_value sym_val = mrbc_symbol_new(vm, (const char*)regs[a].string->data);

  mrbc_decref( &regs[a] );
  regs[a] = sym_val;
}


//================================================================
/*! OP_SYMBOL

  R[a] = intern(Pool[b])
*/
static inline void op_symbol( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  const char *p = (const char *)mrbc_irep_pool_ptr(vm->cur_irep, b);
  mrbc_sym sym_id = mrbc_str_to_symid( p+3 );	// 3 is TT and length

  mrbc_decref(&regs[a]);
  regs[a] = mrbc_symbol_value( sym_id );
}


//================================================================
/*! OP_STRING

  R[a] = str_dup(Pool[b])
*/
static inline void op_string( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);
  regs[a] = mrbc_irep_pool_value(vm, b);
}


//================================================================
/*! OP_STRCAT

  str_cat(R[a],R[a+1])
*/
static inline void op_strcat( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

#if MRBC_USE_STRING
  // call "to_s"
  mrbc_method method;
  if( mrbc_find_method( &method, find_class_by_object(&regs[a+1]),
			MRBC_SYM(to_s)) == 0 ) return;
  if( !method.c_func ) return;		// TODO: Not support?

  method.func( vm, regs + a + 1, 0 );
  mrbc_string_append( &regs[a], &regs[a+1] );
  mrbc_decref_empty( &regs[a+1] );

#else
  mrbc_printf("Not supported!\n");
#endif
}


//================================================================
/*! OP_HASH

  R[a] = hash_new(R[a],R[a+1]..R[a+b*2-1])
*/
static inline void op_hash( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_value value = mrbc_hash_new(vm, b);
  if( value.hash == NULL ) return;   // ENOMEM

  // note: Do not detect duplicate keys.
  b *= 2;
  memcpy( value.hash->data, &regs[a], sizeof(mrbc_value) * b );
  memset( &regs[a], 0, sizeof(mrbc_value) * b );
  value.hash->n_stored = b;

  mrbc_decref(&regs[a]);
  regs[a] = value;
}


//================================================================
/*! OP_HASHADD

  hash_push(R[a],R[a+1]..R[a+b*2])
*/
static inline void op_hashadd( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  int sz1 = mrbc_array_size(&regs[a]);
  int sz2 = b * 2;

  int ret = mrbc_array_resize(&regs[a], sz1 + sz2);
  if( ret != 0 ) return;	// ENOMEM

  // data copy.
  // note: Do not detect duplicate keys.
  memcpy( regs[a].hash->data + sz1, &regs[a+1], sizeof(mrbc_value) * sz2 );
  memset( &regs[a+1], 0, sizeof(mrbc_value) * sz2 );
  regs[a].hash->n_stored = sz1 + sz2;
}


//================================================================
/*! OP_BLOCK, OP_METHOD

  R[a] = lambda(Irep[b],L_BLOCK)
  R[a] = lambda(Irep[b],L_METHOD)
*/
static inline void op_method( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_decref(&regs[a]);

  mrbc_value val = mrbc_proc_new(vm, mrbc_irep_child_irep(vm->cur_irep, b));
  if( !val.proc ) return;	// ENOMEM

  regs[a] = val;
}


//================================================================
/*! OP_RANGE_INC

  R[a] = range_new(R[a],R[a+1],FALSE)
*/
static inline void op_range_inc( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_value value = mrbc_range_new(vm, &regs[a], &regs[a+1], 0);
  regs[a] = value;
  regs[a+1].tt = MRBC_TT_EMPTY;
}


//================================================================
/*! OP_RANGE_EXC

  R[a] = range_new(R[a],R[a+1],TRUE)
*/
static inline void op_range_exc( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_value value = mrbc_range_new(vm, &regs[a], &regs[a+1], 1);
  regs[a] = value;
  regs[a+1].tt = MRBC_TT_EMPTY;
}


//================================================================
/*! OP_CLASS

  R[a] = newclass(R[a],Syms[b],R[a+1])
*/
static inline void op_class( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  const char *class_name = mrbc_irep_symbol_cstr(vm->cur_irep, b);
  mrbc_class *super = (regs[a+1].tt == MRBC_TT_CLASS) ? regs[a+1].cls : 0;
  mrbc_class *cls = mrbc_define_class(vm, class_name, super);
  if( !cls ) return;		// ENOMEM

  // (note)
  //  regs[a] was set to NIL by compiler. So, no need to release regs[a].
  regs[a].tt = MRBC_TT_CLASS;
  regs[a].cls = cls;
}


//================================================================
/*! OP_EXEC

  R[a] = blockexec(R[a],Irep[b])
*/
static inline void op_exec( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();
  assert( regs[a].tt == MRBC_TT_CLASS );

  // prepare callinfo
  mrbc_push_callinfo(vm, 0, a, 0);

  // target irep
  vm->cur_irep = mrbc_irep_child_irep(vm->cur_irep, b);
  vm->inst = vm->cur_irep->inst;
  vm->cur_regs += a;

  vm->target_class = regs[a].cls;
}


//================================================================
/*! OP_DEF

  R[a].newmethod(Syms[b],R[a+1]); R[a] = Syms[b]
*/
static inline void op_def( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  assert( regs[a].tt == MRBC_TT_CLASS );
  assert( regs[a+1].tt == MRBC_TT_PROC );

  mrbc_class *cls = regs[a].cls;
  mrbc_sym sym_id = mrbc_irep_symbol_id(vm->cur_irep, b);
  mrbc_proc *proc = regs[a+1].proc;
  mrbc_method *method;

  if( vm->vm_id == 0 ) {
    method = mrbc_raw_alloc_no_free( sizeof(mrbc_method) );
  } else {
    method = mrbc_raw_alloc( sizeof(mrbc_method) );
  }
  if( !method ) return; // ENOMEM

  method->type = (vm->vm_id == 0) ? 'm' : 'M';
  method->c_func = 0;
  method->sym_id = sym_id;
  method->irep = proc->irep;
  method->next = cls->method_link;
  cls->method_link = method;

  // checking same method
  for( ;method->next != NULL; method = method->next ) {
    if( method->next->sym_id == sym_id ) {
      // Found it. Unchain it in linked list and remove.
      mrbc_method *del_method = method->next;

      method->next = del_method->next;
      if( del_method->type == 'M' ) mrbc_raw_free( del_method );

      break;
    }
  }
}


//================================================================
/*! OP_ALIAS

  alias_method(target_class,Syms[a],Syms[b])
*/
static inline void op_alias( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_BB();

  mrbc_sym sym_id_new = mrbc_irep_symbol_id(vm->cur_irep, a);
  mrbc_sym sym_id_org = mrbc_irep_symbol_id(vm->cur_irep, b);
  mrbc_class *cls = vm->target_class;
  mrbc_method *method = mrbc_raw_alloc( sizeof(mrbc_method) );
  if( !method ) return;	// ENOMEM

  if( mrbc_find_method( method, cls, sym_id_org ) == 0 ) {
    mrbc_printf("NameError: undefined method '%s'\n",
		mrbc_symid_to_str(sym_id_org));
    mrbc_raw_free( method );
    return;
  }

  method->type = 'M';
  method->sym_id = sym_id_new;
  method->next = cls->method_link;
  cls->method_link = method;

  // checking same method
  //  see OP_DEF function. same it.
  for( ;method->next != NULL; method = method->next ) {
    if( method->next->sym_id == sym_id_new ) {
      mrbc_method *del_method = method->next;
      method->next = del_method->next;
      if( del_method->type == 'M' ) mrbc_raw_free( del_method );
      break;
    }
  }
}


//================================================================
/*! OP_SCLASS

  R[a] = R[a].singleton_class
*/
static inline void op_sclass( mrbc_vm *vm, mrbc_value *regs EXT )
{
  // currently, not supported
  FETCH_B();
}


//================================================================
/*! OP_TCLASS

  R[a] = target_class
*/
static inline void op_tclass( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_B();

  mrbc_decref(&regs[a]);
  regs[a].tt = MRBC_TT_CLASS;
  regs[a].cls = vm->target_class;
}


#if !defined(MRBC_SUPPORT_OP_EXT)
//================================================================
/*! OP_EXTn

  make 1st operand (a) 16bit
  make 2nd operand (b) 16bit
  make 2nd operand (b) 16bit
*/
static inline void op_ext( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_Z();
  mrbc_print("Not support op_ext. Re-compile with MRBC_SUPPORT_OP_EXT\n");
}
#endif


//================================================================
/*! OP_STOP

  stop VM
*/
static inline void op_stop( mrbc_vm *vm, mrbc_value *regs EXT )
{
  FETCH_Z();

  vm->flag_preemption = 1;
  vm->flag_stop = 1;
}


//================================================================
/* Unsupported opecodes
*/
static inline void op_unsupported( mrbc_vm *vm )
{
  char buf[40];
  mrbc_snprintf( buf, sizeof(buf), "Unimplemented opcode (0x%02x) found.", *(vm->inst - 1));
  mrbc_raise( vm, MRBC_CLASS(Exception), buf );
}

static inline void op_getsv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_setsv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_getcv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_setcv( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_setmcnst( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_call( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_key_p( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_keyend( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_karg( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_hashcat( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_lambda( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_oclass( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_module( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_undef( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_debug( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
static inline void op_err( mrbc_vm *vm, mrbc_value *regs EXT )
{
  op_unsupported(vm);
}
#undef EXT

//================================================================
/*! Fetch a bytecode and execute

  @param  vm	A pointer to VM.
  @retval 0	(maybe) premption by timer.
  @retval 1	program done.
  @retval 2	exception occurred.
*/
int mrbc_vm_run( struct VM *vm )
{
#if defined(MRBC_SUPPORT_OP_EXT)
  int ext = 0;
#define EXT , ext
#else
#define EXT
#endif

char i = 0;
  while( 1 ) {
    mrbc_value *regs = vm->cur_regs;
    uint8_t op = *vm->inst++;		// Dispatch

  i ++;
  if(i >= 20) i = 0;
  // char buf[64];
  // sprintf(buf, "in vm run, op: %d, 0x%X", op, op);
  // VDP_drawText(buf, 1, i);
    switch( op ) {
    case OP_NOP:        op_nop       (vm, regs EXT); break;
    case OP_MOVE:       op_move      (vm, regs EXT); break;
    case OP_LOADL:      op_loadl     (vm, regs EXT); break;
    case OP_LOADI:      op_loadi     (vm, regs EXT); break;
    case OP_LOADINEG:   op_loadineg  (vm, regs EXT); break;
    case OP_LOADI__1:   // fall through
    case OP_LOADI_0:    // fall through
    case OP_LOADI_1:    // fall through
    case OP_LOADI_2:    // fall through
    case OP_LOADI_3:    // fall through
    case OP_LOADI_4:    // fall through
    case OP_LOADI_5:    // fall through
    case OP_LOADI_6:    // fall through
    case OP_LOADI_7:    op_loadi_n   (vm, regs EXT); break;
    case OP_LOADI16:    op_loadi16   (vm, regs EXT); break;
    case OP_LOADI32:    op_loadi32   (vm, regs EXT); break;
    case OP_LOADSYM:    op_loadsym   (vm, regs EXT); break;
    case OP_LOADNIL:    op_loadnil   (vm, regs EXT); break;
    case OP_LOADSELF:   op_loadself  (vm, regs EXT); break;
    case OP_LOADT:      op_loadt     (vm, regs EXT); break;
    case OP_LOADF:      op_loadf     (vm, regs EXT); break;
    case OP_GETGV:      op_getgv     (vm, regs EXT); break;
    case OP_SETGV:      op_setgv     (vm, regs EXT); break;
    case OP_GETSV:      op_getsv     (vm, regs EXT); break; // not implemented.
    case OP_SETSV:      op_setsv     (vm, regs EXT); break; // not implemented.
    case OP_GETIV:      op_getiv     (vm, regs EXT); break;
    case OP_SETIV:      op_setiv     (vm, regs EXT); break;
    case OP_GETCV:      op_getcv     (vm, regs EXT); break; // not implemented.
    case OP_SETCV:      op_setcv     (vm, regs EXT); break; // not implemented.
    case OP_GETCONST:   op_getconst  (vm, regs EXT); break;
    case OP_SETCONST:   op_setconst  (vm, regs EXT); break;
    case OP_GETMCNST:   op_getmcnst  (vm, regs EXT); break;
    case OP_SETMCNST:   op_setmcnst  (vm, regs EXT); break; // not implemented.
    case OP_GETUPVAR:   op_getupvar  (vm, regs EXT); break;
    case OP_SETUPVAR:   op_setupvar  (vm, regs EXT); break;
    case OP_GETIDX:     op_getidx    (vm, regs EXT); break;
    case OP_SETIDX:     op_setidx    (vm, regs EXT); break;
    case OP_JMP:        op_jmp       (vm, regs EXT); break;
    case OP_JMPIF:      op_jmpif     (vm, regs EXT); break;
    case OP_JMPNOT:     op_jmpnot    (vm, regs EXT); break;
    case OP_JMPNIL:     op_jmpnil    (vm, regs EXT); break;
    case OP_JMPUW:      op_jmpuw     (vm, regs EXT); break;
    case OP_EXCEPT:     op_except    (vm, regs EXT); break;
    case OP_RESCUE:     op_rescue    (vm, regs EXT); break;
    case OP_RAISEIF:    op_raiseif   (vm, regs EXT); break;
    case OP_SSEND:      op_ssend     (vm, regs EXT); break;
    case OP_SSENDB:     op_ssendb    (vm, regs EXT); break;
    case OP_SEND:       op_send      (vm, regs EXT); break;
    case OP_SENDB:      op_sendb     (vm, regs EXT); break;
    case OP_CALL:       op_call      (vm, regs EXT); break; // not implemented.
    case OP_SUPER:      op_super     (vm, regs EXT); break;
    case OP_ARGARY:     op_argary    (vm, regs EXT); break;
    case OP_ENTER:      op_enter     (vm, regs EXT); break;
    case OP_KEY_P:      op_key_p     (vm, regs EXT); break; // not implemented.
    case OP_KEYEND:     op_keyend    (vm, regs EXT); break; // not implemented.
    case OP_KARG:       op_karg      (vm, regs EXT); break; // not implemented.
    case OP_RETURN:     op_return    (vm, regs EXT); break;
    case OP_RETURN_BLK: op_return_blk(vm, regs EXT); break;
    case OP_BREAK:      op_break     (vm, regs EXT); break;
    case OP_BLKPUSH:    op_blkpush   (vm, regs EXT); break;
    case OP_ADD:        op_add       (vm, regs EXT); break;
    case OP_ADDI:       op_addi      (vm, regs EXT); break;
    case OP_SUB:        op_sub       (vm, regs EXT); break;
    case OP_SUBI:       op_subi      (vm, regs EXT); break;
    case OP_MUL:        op_mul       (vm, regs EXT); break;
    case OP_DIV:        op_div       (vm, regs EXT); break;
    case OP_EQ:         op_eq        (vm, regs EXT); break;
    case OP_LT:         op_lt        (vm, regs EXT); break;
    case OP_LE:         op_le        (vm, regs EXT); break;
    case OP_GT:         op_gt        (vm, regs EXT); break;
    case OP_GE:         op_ge        (vm, regs EXT); break;
    case OP_ARRAY:      op_array     (vm, regs EXT); break;
    case OP_ARRAY2:     op_array2    (vm, regs EXT); break;
    case OP_ARYCAT:     op_arycat    (vm, regs EXT); break;
    case OP_ARYPUSH:    op_arypush   (vm, regs EXT); break;
    case OP_ARYDUP:     op_arydup    (vm, regs EXT); break;
    case OP_AREF:       op_aref      (vm, regs EXT); break;
    case OP_ASET:       op_aset      (vm, regs EXT); break;
    case OP_APOST:      op_apost     (vm, regs EXT); break;
    case OP_INTERN:     op_intern    (vm, regs EXT); break;
    case OP_SYMBOL:     op_symbol    (vm, regs EXT); break;
    case OP_STRING:     op_string    (vm, regs EXT); break;
    case OP_STRCAT:     op_strcat    (vm, regs EXT); break;
    case OP_HASH:       op_hash      (vm, regs EXT); break;
    case OP_HASHADD:    op_hashadd   (vm, regs EXT); break;
    case OP_HASHCAT:    op_hashcat   (vm, regs EXT); break; // not implemented.
    case OP_LAMBDA:     op_lambda    (vm, regs EXT); break; // not implemented.
    case OP_BLOCK:      // fall through
    case OP_METHOD:     op_method    (vm, regs EXT); break;
    case OP_RANGE_INC:  op_range_inc (vm, regs EXT); break;
    case OP_RANGE_EXC:  op_range_exc (vm, regs EXT); break;
    case OP_OCLASS:     op_oclass    (vm, regs EXT); break; // not implemented.
    case OP_CLASS:      op_class     (vm, regs EXT); break;
    case OP_MODULE:     op_module    (vm, regs EXT); break; // not implemented.
    case OP_EXEC:       op_exec      (vm, regs EXT); break;
    case OP_DEF:        op_def       (vm, regs EXT); break;
    case OP_ALIAS:      op_alias     (vm, regs EXT); break;
    case OP_UNDEF:      op_undef     (vm, regs EXT); break; // not implemented.
    case OP_SCLASS:     op_sclass    (vm, regs EXT); break;
    case OP_TCLASS:     op_tclass    (vm, regs EXT); break;
    case OP_DEBUG:      op_debug     (vm, regs EXT); break; // not implemented.
    case OP_ERR:        op_err       (vm, regs EXT); break; // not implemented.
#if defined(MRBC_SUPPORT_OP_EXT)
    case OP_EXT1:       ext = 1; continue;
    case OP_EXT2:       ext = 2; continue;
    case OP_EXT3:       ext = 3; continue;
#else
    case OP_EXT1:       // fall through
    case OP_EXT2:       // fall through
    case OP_EXT3:       op_ext       (vm, regs EXT); break;
#endif
    case OP_STOP:       op_stop      (vm, regs EXT); break;
    default:		op_unsupported(vm);          break;
    } // end switch.

#undef EXT
#if defined(MRBC_SUPPORT_OP_EXT)
    ext = 0;
#endif
    if( !vm->flag_preemption ) continue;	// execute next ope code.
    if( !mrbc_israised(vm) ) return vm->flag_stop; // normal return.


    // Handle exception
    vm->flag_preemption = 0;
    const mrbc_irep_catch_handler *handler;

    while( 1 ) {
      const mrbc_irep *irep = vm->cur_irep;
      const mrbc_irep_catch_handler *catch_table =
	(const mrbc_irep_catch_handler *)(irep->inst + irep->ilen);
      uint32_t inst = vm->inst - irep->inst;
      int cnt = irep->clen;

      for( cnt--; cnt >= 0 ; cnt-- ) {
	handler = catch_table + cnt;
	if( (bin_to_uint32(handler->begin) < inst) &&
	    (inst <= bin_to_uint32(handler->end)) ) goto JUMP_TO_HANDLER;
      }

      if( !vm->callinfo_tail ) return 2;	// return due to exception.
      mrbc_pop_callinfo( vm );
    }

  JUMP_TO_HANDLER:
    // jump to handler (rescue or ensure).
    vm->inst = vm->cur_irep->inst + bin_to_uint32(handler->target);
  }
}

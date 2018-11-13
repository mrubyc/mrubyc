/*! @file
  @brief
  mruby bytecode executor.

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Fetch mruby VM bytecodes, decode and execute.

  </pre>
*/

#include "vm_config.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "vm.h"
#include "alloc.h"
#include "load.h"
#include "static.h"
#include "global.h"
#include "mruby/opcode.h"
#include "class.h"
#include "symbol.h"
#include "console.h"

#include "c_string.h"
#include "c_range.h"
#include "c_array.h"
#include "c_hash.h"

#include <stdio.h>

mrbc_static_method(Proc, call);


static uint32_t free_vm_bitmap[MAX_VM_COUNT / 32 + 1];
#define FREE_BITMAP_WIDTH 32
#define Num(n) (sizeof(n)/sizeof((n)[0]))


//================================================================
/*! Number of leading zeros.

  @param	x	target (32bit unsined)
  @retval	int	nlz value
*/
static inline int nlz32(uint32_t x)
{
  if( x == 0 ) return 32;

  int n = 1;
  if((x >> 16) == 0 ) { n += 16; x <<= 16; }
  if((x >> 24) == 0 ) { n +=  8; x <<=  8; }
  if((x >> 28) == 0 ) { n +=  4; x <<=  4; }
  if((x >> 30) == 0 ) { n +=  2; x <<=  2; }
  return n - (x >> 31);
}


//================================================================
/*! get sym[n] from symbol table in irep

  @param  p	Pointer to IREP SYMS section.
  @param  n	n th
  @return	symbol name string
*/
const char * mrbc_get_irep_symbol( const uint8_t *p, int n )
{
  int cnt = bin_to_uint32(p);
  if( n >= cnt ) return 0;
  p += 4;
  while( n > 0 ) {
    uint16_t s = bin_to_uint16(p);
    p += 2+s+1;   // size(2 bytes) + symbol len + '\0'
    n--;
  }
  return (char *)p+2;  // skip size(2 bytes)
}


//================================================================
/*! get callee name

  @param  vm	Pointer to VM
  @return	string
*/
const char *mrbc_get_callee_name( struct VM *vm )
{
  return symid_to_str(vm->current_mid);
}


//================================================================
/*!@brief

*/
static void not_supported(void)
{
  console_printf("Not supported!\n");
}


//================================================================
/*! mrbc_irep allocator

  @param  vm	Pointer to VM.
  @return	Pointer to allocated memory or NULL.
*/
mrbc_irep *mrbc_irep_alloc(struct VM *vm)
{
  mrbc_irep *p = (mrbc_irep *)mrbc_alloc(vm, sizeof(mrbc_irep));
  if( p )
    memset(p, 0, sizeof(mrbc_irep));	// caution: assume NULL is zero.
  return p;
}


//================================================================
/*! release mrbc_irep holds memory

  @param  irep	Pointer to allocated mrbc_irep.
*/
void mrbc_irep_free(mrbc_irep *irep)
{
  int i;

  // release pools.
  for( i = 0; i < irep->plen; i++ ) {
    mrbc_raw_free( irep->pools[i] );
  }
  if( irep->plen ) mrbc_raw_free( irep->pools );

  // release child ireps.
  for( i = 0; i < irep->rlen; i++ ) {
    mrbc_irep_free( irep->reps[i] );
  }
  if( irep->rlen ) mrbc_raw_free( irep->reps );

  mrbc_raw_free( irep );
}


//================================================================
/*! Push current status to callinfo stack

*/
void mrbc_push_callinfo( struct VM *vm, mrbc_sym mid, int n_args )
{
  mrbc_callinfo *callinfo = mrbc_alloc(vm, sizeof(mrbc_callinfo));
  if( !callinfo ) return;

  callinfo->current_regs = vm->current_regs;
  callinfo->pc_irep = vm->pc_irep;
  callinfo->pc = vm->pc;
  callinfo->mid = mid;
  callinfo->n_args = n_args;
  callinfo->target_class = vm->target_class;
  callinfo->prev = vm->callinfo_tail;
  vm->callinfo_tail = callinfo;
}


//================================================================
/*! Pop current status to callinfo stack

*/
void mrbc_pop_callinfo( struct VM *vm )
{
  mrbc_callinfo *callinfo = vm->callinfo_tail;
  vm->callinfo_tail = callinfo->prev;
  vm->current_regs = callinfo->current_regs;
  vm->pc_irep = callinfo->pc_irep;
  vm->pc = callinfo->pc;
  vm->target_class = callinfo->target_class;

  mrbc_free(vm, callinfo);
}




//================================================================
/*!@brief
  Execute OP_NOP

  No operation

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_nop( mrbc_vm *vm, mrbc_value *regs )
{
  return 0;
}


//================================================================
/*!@brief
  Execute OP_MOVE

  R(A) := R(B)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_move( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_release(&regs[ra]);
  mrbc_dup(&regs[rb]);
  regs[ra] = regs[rb];

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADL

  R(A) := Pool(Bx)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadl( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_release(&regs[ra]);
  assert(rb < vm->pc_irep->plen);
  regs[ra] = *vm->pc_irep->pools[rb];

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADI

  R(A) := sBx

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadi( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint8_t val )
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = val;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADSYM

  R(A) := Syms(Bx)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadsym( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);

  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_SYMBOL;
  regs[ra].i = sym_id;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADNIL

  R(A) := nil

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadnil( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_NIL;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADSELF

  R(A) := self

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadself( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_release(&regs[ra]);
  mrbc_dup(&regs[0]);
  regs[ra] = regs[0];

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADT

  R(A) := true

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadt( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_TRUE;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LOADF

  R(A) := false

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_loadf( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FALSE;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_GETGLOBAL

  R(A) := getglobal(Syms(Bx))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_getglobal( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);

  mrbc_release(&regs[ra]);
  mrbc_value *v = mrbc_get_global(sym_id);
  if( v == NULL ) {
    regs[ra] = mrbc_nil_value();
  } else {
    mrbc_dup(v);
    regs[ra] = *v;
  }

  return 0;
}


//================================================================
/*!@brief
  Execute OP_SETGLOBAL

  setglobal(Syms(Bx), R(A))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_setglobal( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);
  mrbc_dup(&regs[ra]);
  mrbc_set_global(sym_id, &regs[ra]);

  return 0;
}


//================================================================
/*!@brief
  Execute OP_GETIV

  R(A) := ivget(Syms(Bx))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_getiv( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name+1);	// skip '@'

  mrbc_value val = mrbc_instance_getiv(&regs[0], sym_id);

  mrbc_release(&regs[ra]);
  regs[ra] = val;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_SETIV

  ivset(Syms(Bx),R(A))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_setiv( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name+1);	// skip '@'

  mrbc_instance_setiv(&regs[0], sym_id, &regs[ra]);

  return 0;
}


//================================================================
/*!@brief
  Execute OP_GETCONST

  R(A) := constget(Syms(Bx))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_getconst( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);

  mrbc_release(&regs[ra]);
  mrbc_value *v = mrbc_get_const(sym_id);
  if( v == NULL ) {		// raise?
    console_printf( "NameError: uninitialized constant %s\n",
		    symid_to_str( sym_id ));
    return 0;
  }

  mrbc_dup(v);
  regs[ra] = *v;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_SETCONST

  constset(Syms(Bx),R(A))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/

static inline int op_setconst( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb ) {
  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);
  mrbc_dup(&regs[ra]);
  mrbc_set_const(sym_id, &regs[ra]);

  return 0;
}



//================================================================
/*!@brief
  Execute OP_GETUPVAR

  R(A) := uvget(B,C)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_getupvar( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb, uint8_t rc )
{
  mrbc_callinfo *callinfo = vm->callinfo_tail;

  // find callinfo
  int n = rc * 2 + 1;
  while( n > 0 ){
    callinfo = callinfo->prev;
    n--;
  }

  mrbc_value *up_regs = callinfo->current_regs;

  mrbc_release( &regs[ra] );
  mrbc_dup( &up_regs[rb] );
  regs[ra] = up_regs[rb];

  return 0;
}



//================================================================
/*!@brief
  Execute OP_SETUPVAR

  uvset(B,C,R(A))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_setupvar( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb, uint8_t rc )
{
  mrbc_callinfo *callinfo = vm->callinfo_tail;

  // find callinfo
  int n = rc * 2 + 1;
  while( n > 0 ){
    callinfo = callinfo->prev;
    n--;
  }

  mrbc_value *up_regs = callinfo->current_regs;

  mrbc_release( &up_regs[rb] );
  mrbc_dup( &regs[ra] );
  up_regs[rb] = regs[ra];

  return 0;
}



//================================================================
/*!@brief
  Execute OP_JMP

  pc += sBx

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_jmp( mrbc_vm *vm, mrbc_value *regs, uint32_t a )
{
  vm->pc = vm->pc_irep->code + a;
  return 0;
}


//================================================================
/*!@brief
  Execute OP_JMPIF

  if R(A) pc += sBx

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_jmpif( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  if( regs[ra].tt > MRBC_TT_FALSE ) {
    vm->pc = vm->pc_irep->code + rb;
  }
  return 0;
}


//================================================================
/*!@brief
  Execute OP_JMPNOT

  if not R(A) pc += sBx

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_jmpnot( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  if( regs[ra].tt <= MRBC_TT_FALSE ) {
    vm->pc = vm->pc_irep->code + rb;
  }
  return 0;
}

//================================================================
/*!@brief
  Execute OP_SEND / OP_SENDB

  OP_SEND   R(A) := call(R(A),Syms(B),R(A+1),...,R(A+C))
  OP_SENDB  R(A) := call(R(A),Syms(B),R(A+1),...,R(A+C),&R(A+C+1))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_send_raw( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb, uint8_t rc )
{
  // rb: index of method sym
  // rc: number of params
  mrbc_value recv = regs[ra];

  // Block param
  int bidx = ra + rc + 1;

  const char *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);
  mrbc_proc *m = find_method(vm, &recv, sym_id);

  if( m == 0 ) {
    mrbc_class *cls = find_class_by_object( vm, &recv );
    console_printf("No method. Class:%s Method:%s\n",
		   symid_to_str(cls->sym_id), sym_name );
    return 0;
  }

  // m is C func
  if( m->c_func ) {
    vm->current_mid = sym_id;
    m->func(vm, regs + ra, rc);

    extern void c_proc_call(mrbc_vm *vm, mrbc_value v[], int argc);
    if( m->func == mrbc_static_method_sym(Proc, call) ) return 0;

    int release_reg = ra+1;
    while( release_reg <= bidx ) {
      mrbc_release(&regs[release_reg]);
      release_reg++;
    }
    return 0;
  }

  // m is Ruby method.
  // callinfo
  mrbc_push_callinfo(vm, sym_id, rc);

  // target irep
  vm->pc_irep = m->irep;
  vm->pc = vm->pc_irep->code;

  // new regs
  vm->current_regs += ra;

  return 0;
}

static inline int op_send_noblk(mrb_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb, uint8_t rc)
{
  int bidx = ra + rc + 1;
  mrbc_release( &regs[bidx] );
  regs[bidx].tt = MRBC_TT_NIL;
  return op_send_raw(vm, regs, ra, rb, rc);
}

static inline int op_send_blk(mrb_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb, uint8_t rc)
{
  int bidx = ra + rc + 1;
  // set Proc object
  if( regs[bidx].tt != MRBC_TT_NIL && regs[bidx].tt != MRBC_TT_PROC ){
    // TODO: fix the following behavior
    // convert to Proc ?
    // raise exceprion in mruby/c ?
    return 0;
  }
  return op_send_raw(vm, regs, ra, rb, rc);
}

//================================================================
/*!@brief
  Execute OP_CALL

  R(A) := self.call(frame.argc, frame.argv)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_call( mrbc_vm *vm, mrbc_value *regs )
{
  mrbc_push_callinfo(vm, 0, 0);

  // jump to proc
  vm->pc_irep = regs[0].proc->irep;
  vm->pc = vm->pc_irep->code;

  return 0;
}



//================================================================
/*!@brief
  Execute OP_SUPER

  R(A) := super(R(A+1),... ,R(A+C+1))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
inline static int op_super( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint8_t rc )
{
  // rc: number of params

  // copy self, same as LOADSELF
  mrbc_release(&regs[ra]);
  mrbc_dup(&regs[0]);
  regs[ra] = regs[0];

  mrbc_sym sym_id = vm->callinfo_tail->mid;

  // find super method
  mrbc_proc *m = 0;
  mrbc_class *cls = regs[ra].instance->cls->super;
  while( cls != 0 ) {
    mrbc_proc *proc = cls->procs;
    while( proc != 0 ) {
      if( proc->sym_id == sym_id ) {
	m = proc;
	goto FIND_SUPER_EXIT;
      }
      proc = proc->next;
    }
    cls = cls->super;
  }
 FIND_SUPER_EXIT:

  if( m == 0 ) {
    // No super method
    return 0;
  }

  // Change class
  regs[ra].instance->cls = cls;

  // m is C func
  if( m->c_func ) {
    m->func(vm, regs + ra, rc);

    extern void c_proc_call(mrbc_vm *vm, mrbc_value v[], int argc);
    if( m->func == mrbc_static_method_sym(Proc, call) ) return 0;

    unsigned int release_reg = ra+1;
    while( release_reg <= ra+rc+1 ) {
      mrbc_release(&regs[release_reg]);
      release_reg++;
    }
    return 0;
  }

  // m is Ruby method.
  // callinfo
  mrbc_push_callinfo(vm, sym_id, rc);

  // target irep
  vm->pc_irep = m->irep;
  vm->pc = vm->pc_irep->code;
  // new regs
  vm->current_regs += ra;

  return 0;
}



//================================================================
/*!@brief
  Execute OP_ARGARY

  R(A) := argument array (16=6:1:5:4)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
inline static int op_argary( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  //

  return 0;
}



//================================================================
/*!@brief
  Execute OP_ENTER

  arg setup according to flags (23=5:5:1:5:5:1:1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_enter( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_callinfo *callinfo = vm->callinfo_tail;
  uint32_t enter_param = ra;
  int def_args = (enter_param >> 13) & 0x1f;  // default args
  int args = (enter_param >> 18) & 0x1f;      // given args
  if( def_args > 0 ){
    vm->pc += callinfo->n_args - args;
  }
  return 0;
}


//================================================================
/*!@brief
  Execute OP_RETURN

  return R(A) (B=normal,in-block return/break)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_return( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  // ra: return value

  mrbc_value *regs0 = regs;
  // return value stored in original regs[0] if return in block
  mrbc_callinfo *ci = vm->callinfo_tail;
  if( ci && ci->current_regs[1].tt == MRBC_TT_PROC ){
    regs0 = regs - 2;
  } 
  mrbc_release(regs0);
  *regs0 = regs[ra];
  regs[ra].tt = MRBC_TT_EMPTY;
    
  // nregs to release
  int nregs = vm->pc_irep->nregs;

  // restore irep,pc,regs
  mrbc_callinfo *callinfo = vm->callinfo_tail;
  if (!callinfo) {
    return;
  }
  vm->callinfo_tail = callinfo->prev;
  vm->current_regs = callinfo->current_regs;
  vm->pc_irep = callinfo->pc_irep;
  vm->pc = callinfo->pc;
  vm->target_class = callinfo->target_class;

  // clear stacked arguments
  int i;
  for( i = 1; i < nregs; i++ ) {
    mrbc_release( &regs[i] );
  }

  // release callinfo
  mrbc_free(vm, callinfo);

  return 0;
}


//================================================================
/*!@brief
  Execute OP_BLKPUSH

  R(A) := block (16=6:1:5:4)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_blkpush( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_value *stack = regs + 1;

  if( stack[0].tt == MRBC_TT_NIL ){
    return -1;  // EYIELD
  }

  mrbc_release(&regs[ra]);
  mrbc_dup( stack );
  regs[ra] = stack[0];

  return 0;
}



//================================================================
/*!@brief
  Execute OP_ADD

  R(A) := R(A)+R(A+1) (Syms[B]=:+,C=1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_add( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Fixnum, Fixnum
      regs[ra].i += regs[ra+1].i;
      return 0;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Fixnum, Float
      regs[ra].tt = MRBC_TT_FLOAT;
      regs[ra].d = regs[ra].i + regs[ra+1].d;
      return 0;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Float, Fixnum
      regs[ra].d += regs[ra+1].i;
      return 0;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Float, Float
      regs[ra].d += regs[ra+1].d;
      return 0;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  return 0;
}


//================================================================
/*!@brief
  Execute OP_ADDI

  R(A) := R(A)+B

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_addi( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
 if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    regs[ra].i += rb;
    return 0;
  }

#if MRBC_USE_FLOAT
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    regs[ra].d += rb;
    return 0;
  }
#endif

  not_supported();
  return 0;
}


//================================================================
/*!@brief
  Execute OP_SUB

  R(A) := R(A)-R(A+1) (Syms[B]=:-,C=1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_sub( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Fixnum, Fixnum
      regs[ra].i -= regs[ra+1].i;
      return 0;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Fixnum, Float
      regs[ra].tt = MRBC_TT_FLOAT;
      regs[ra].d = regs[ra].i - regs[ra+1].d;
      return 0;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Float, Fixnum
      regs[ra].d -= regs[ra+1].i;
      return 0;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Float, Float
      regs[ra].d -= regs[ra+1].d;
      return 0;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;
}


//================================================================
/*!@brief
  Execute OP_SUBI

  R(A) := R(A)-B

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_subi( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    regs[ra].i -= rb;
    return 0;
  }

#if MRBC_USE_FLOAT
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    regs[ra].d -= rb;
    return 0;
  }
#endif

  not_supported();
  return 0;
}


//================================================================
/*!@brief
  Execute OP_MUL

  R(A) := R(A)*R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_mul( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Fixnum, Fixnum
      regs[ra].i *= regs[ra+1].i;
      return 0;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Fixnum, Float
      regs[ra].tt = MRBC_TT_FLOAT;
      regs[ra].d = regs[ra].i * regs[ra+1].d;
      return 0;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Float, Fixnum
      regs[ra].d *= regs[ra+1].i;
      return 0;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Float, Float
      regs[ra].d *= regs[ra+1].d;
      return 0;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;
}


//================================================================
/*!@brief
  Execute OP_DIV

  R(A) := R(A)/R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_div( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Fixnum, Fixnum
      regs[ra].i /= regs[ra+1].i;
      return 0;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Fixnum, Float
      regs[ra].tt = MRBC_TT_FLOAT;
      regs[ra].d = regs[ra].i / regs[ra+1].d;
      return 0;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {	// in case of Float, Fixnum
      regs[ra].d /= regs[ra+1].i;
      return 0;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {	// in case of Float, Float
      regs[ra].d /= regs[ra+1].d;
      return 0;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;
}


//================================================================
/*!@brief
  Execute OP_EQ

  R(A) := R(A)==R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_eq( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  int result = mrbc_compare(&regs[ra], &regs[ra+1]);

  mrbc_release(&regs[ra+1]);
  mrbc_release(&regs[ra]);
  regs[ra].tt = result ? MRBC_TT_FALSE : MRBC_TT_TRUE;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LT

  R(A) := R(A)<R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_lt( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  int result;

  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].i < regs[ra+1].i;	// in case of Fixnum, Fixnum
      goto DONE;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].i < regs[ra+1].d;	// in case of Fixnum, Float
      goto DONE;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].d < regs[ra+1].i;	// in case of Float, Fixnum
      goto DONE;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].d < regs[ra+1].d;	// in case of Float, Float
      goto DONE;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;

DONE:
  regs[ra].tt = result ? MRBC_TT_TRUE : MRBC_TT_FALSE;
  return 0;
}


//================================================================
/*!@brief
  Execute OP_LE

  R(A) := R(A)<=R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_le( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  int result;

  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].i <= regs[ra+1].i;	// in case of Fixnum, Fixnum
      goto DONE;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].i <= regs[ra+1].d;	// in case of Fixnum, Float
      goto DONE;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].d <= regs[ra+1].i;	// in case of Float, Fixnum
      goto DONE;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].d <= regs[ra+1].d;	// in case of Float, Float
      goto DONE;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;

DONE:
  regs[ra].tt = result ? MRBC_TT_TRUE : MRBC_TT_FALSE;
  return 0;
}


//================================================================
/*!@brief
  Execute OP_GT

  R(A) := R(A)>=R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_gt( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  int result;

  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].i > regs[ra+1].i;	// in case of Fixnum, Fixnum
      goto DONE;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].i > regs[ra+1].d;	// in case of Fixnum, Float
      goto DONE;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].d > regs[ra+1].i;	// in case of Float, Fixnum
      goto DONE;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].d > regs[ra+1].d;	// in case of Float, Float
      goto DONE;
    }
#endif
  }

  // other case
  op_send_noblk(vm , regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;

DONE:
  regs[ra].tt = result ? MRBC_TT_TRUE : MRBC_TT_FALSE;
  return 0;
}


//================================================================
/*!@brief
  Execute OP_GE

  R(A) := R(A)>=R(A+1)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_ge( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  int result;

  if( regs[ra].tt == MRBC_TT_FIXNUM ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].i >= regs[ra+1].i;	// in case of Fixnum, Fixnum
      goto DONE;
    }
#if MRBC_USE_FLOAT
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].i >= regs[ra+1].d;	// in case of Fixnum, Float
      goto DONE;
    }
  }
  if( regs[ra].tt == MRBC_TT_FLOAT ) {
    if( regs[ra+1].tt == MRBC_TT_FIXNUM ) {
      result = regs[ra].d >= regs[ra+1].i;	// in case of Float, Fixnum
      goto DONE;
    }
    if( regs[ra+1].tt == MRBC_TT_FLOAT ) {
      result = regs[ra].d >= regs[ra+1].d;	// in case of Float, Float
      goto DONE;
    }
#endif
  }

  // other case
  op_send_noblk(vm, regs, ra, ra, 1);
  mrbc_release(&regs[ra+1]);
  return 0;

DONE:
  regs[ra].tt = result ? MRBC_TT_TRUE : MRBC_TT_FALSE;
  return 0;
}


//================================================================
/*!@brief
  Create Array object

  R(A) := ary_new(R(B),R(B+1)..R(B+C))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_array( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_value value = mrbc_array_new(vm, rb);
  if( value.array == NULL ) return -1;	// ENOMEM

  memcpy( value.array->data, &regs[ra], sizeof(mrbc_value) * rb );
  memset( &regs[ra], 0, sizeof(mrbc_value) * rb );
  value.array->n_stored = rb;

  mrbc_release(&regs[ra]);
  regs[ra] = value;

  return 0;
}


//================================================================
/*!@brief
  Create string object

  R(A) := str_dup(Lit(Bx))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_string( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
#if MRBC_USE_STRING
  mrbc_object *pool_obj = vm->pc_irep->pools[rb];

  /* CAUTION: pool_obj->str - 2. see IREP POOL structure. */
  int len = bin_to_uint16(pool_obj->str - 2);
  mrbc_value value = mrbc_string_new(vm, pool_obj->str, len);
  if( value.string == NULL ) return -1;		// ENOMEM

  mrbc_release(&regs[ra]);
  regs[ra] = value;

#else
  not_supported();
#endif
  return 0;
}


//================================================================
/*!@brief
  String Catination

  str_cat(R(A),R(B))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_strcat( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
#if MRBC_USE_STRING
  // call "to_s"
  mrbc_sym sym_id = str_to_symid("to_s");
  mrbc_proc *m = find_method(vm, &regs[ra + 1], sym_id);
  if( m && m->c_func ){
    m->func(vm, regs+ra, 0);
  }

  mrbc_value v = mrbc_string_add(vm, &regs[ra], &regs[ra + 1]);
  mrbc_release(&regs[ra]);
  regs[ra] = v;

#else
  not_supported();
#endif
  return 0;
}


//================================================================
/*!@brief
  Create Hash object

  R(A) := hash_new(R(B),R(B+1)..R(B+C))

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_hash( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_value value = mrbc_hash_new(vm, rb);
  if( value.hash == NULL ) return -1;	// ENOMEM

  rb *= 2;
  memcpy( value.hash->data, &regs[ra], sizeof(mrbc_value) * rb );
  memset( &regs[ra], 0, sizeof(mrbc_value) * rb );
  value.hash->n_stored = rb;

  mrbc_release(&regs[ra]);
  regs[ra] = value;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_LAMBDA

  R(A) := lambda(SEQ[Bz],Cz)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_lambda( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  // rb: sequence position in irep list
  // int c = GETARG_C(code);    // TODO: Add flags support for OP_LAMBDA
  mrbc_proc *proc = mrbc_rproc_alloc(vm, "(lambda)");

  proc->c_func = 0;
  proc->irep = vm->pc_irep->reps[rb];

  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_PROC;
  regs[ra].proc = proc;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_RANGE

  R(A) := range_new(R(B),R(B+1),C)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_range_inc( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_dup(&regs[ra]);
  mrbc_dup(&regs[ra+1]);

  mrbc_value value = mrbc_range_new(vm, &regs[ra], &regs[ra+1], 0);
  if( value.range == NULL ) return -1;		// ENOMEM

  mrbc_release(&regs[ra]);
  regs[ra] = value;

  return 0;
}

static inline int op_range_exc( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_dup(&regs[ra]);
  mrbc_dup(&regs[ra+1]);

  mrbc_value value = mrbc_range_new(vm, &regs[ra], &regs[ra+1], 1);
  if( value.range == NULL ) return -1;		// ENOMEM

  mrbc_release(&regs[ra]);
  regs[ra] = value;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_CLASS

    R(A) := newclass(R(A),Syms(B),R(A+1))
    Syms(B): class name
    R(A+1): super class

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_class( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_irep *cur_irep = vm->pc_irep;
  const char *sym_name = mrbc_get_irep_symbol(cur_irep->ptr_to_sym, rb);
  mrbc_class *super = (regs[ra+1].tt == MRBC_TT_CLASS) ? regs[ra+1].cls : mrbc_class_object;

  mrbc_class *cls = mrbc_define_class(vm, sym_name, super);

  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_CLASS;
  regs[ra].cls = cls;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_EXEC

  R(A) := blockexec(R(A),SEQ[Bx])

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_exec( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_value recv = regs[ra];

  // prepare callinfo
  mrbc_push_callinfo(vm, 0, 0);

  // target irep
  vm->pc_irep = vm->irep->reps[rb];
  vm->pc = vm->pc_irep->code;

  // new regs
  vm->current_regs += ra;

  vm->target_class = find_class_by_object(vm, &recv);

  return 0;
}



//================================================================
/*!@brief
  Execute OP_METHOD

  R(a) = lambda(SEQ[b],L_METHOD)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_method( mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb )
{
  mrbc_proc *proc = mrbc_rproc_alloc(vm, "(method)");
  proc->tt = MRBC_TT_PROC;
  proc->c_func = 0;
  proc->irep = vm->pc_irep->reps[rb];

  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_PROC;
  regs[ra].proc = proc;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_TCLASS

  R(A) := target_class

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval 0  No error.
*/
static inline int op_tclass( mrbc_vm *vm, mrbc_value *regs, uint32_t ra )
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_CLASS;
  regs[ra].cls = vm->target_class;

  return 0;
}


//================================================================
/*!@brief
  Execute OP_STOP and OP_ABORT

  stop VM (OP_STOP)
  stop VM without release memory (OP_ABORT)

  @param  vm    A pointer of VM.
  @param  code  bytecode
  @param  regs  vm->regs + vm->reg_top
  @retval -1  No error and exit from vm.
*/
static inline int op_stop( mrbc_vm *vm, mrbc_value *regs )
{
  int i;
  for( i = 0; i < MAX_REGS_SIZE; i++ ) {
    mrbc_release(&vm->regs[i]);
  }

  vm->flag_preemption = 1;

  mrbc_irep_free( vm->irep );
  vm->irep = NULL;

  return -1;
}

static inline int op_abort( mrbc_vm *vm, mrbc_value *regs )
{
  vm->flag_preemption = 1;

  return -1;
}

// R(a).newmethod(Syms(b),R(a+1))
static inline int op_DEF(mrbc_vm *vm, mrbc_value *regs, uint32_t ra, uint16_t rb)
{
  if (regs[ra].tt != MRBC_TT_CLASS) {
    return -1;
  }
  if (regs[ra + 1].tt != MRBC_TT_PROC) {
    return -1;
  }

  mrbc_class *cls = regs[ra].cls;
  mrbc_proc *proc = regs[ra + 1].proc;
  mrbc_proc *p = cls->procs;
  mrbc_proc **pp = &cls->procs;
  char const *sym_name = mrbc_get_irep_symbol(vm->pc_irep->ptr_to_sym, rb);
  mrbc_sym sym_id = str_to_symid(sym_name);

  while( p != NULL ) {
    if( p->sym_id == sym_id ) break;
    pp = &p->next;
    p = p->next;
  }
  if( p ) {
    // found it.
    *pp = p->next;
    if( !p->c_func ) {
      mrbc_value v = {.tt = MRBC_TT_PROC};
      v.proc = p;
      mrbc_release(&v);
    }
  }

  // add proc to class
  proc->c_func = 0;
  proc->sym_id = sym_id;
#ifdef MRBC_DEBUG
  proc->names = sym_name;		// debug only.
#endif
  proc->next = cls->procs;
  cls->procs = proc;
  proc->ref_count++;

  return 0;
}

static inline int op_LOADI__1(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = -1;
  return 0;
}

static inline int op_LOADI_0(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 0;
  return 0;
}

static inline int op_LOADI_1(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 1;
  return 0;
}

static inline int op_LOADI_2(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 2;
  return 0;
}

static inline int op_LOADI_3(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 3;
  return 0;
}

static inline int op_LOADI_4(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 4;
  return 0;
}

static inline int op_LOADI_5(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 5;
  return 0;
}

static inline int op_LOADI_6(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 6;
  return 0;
}

static inline int op_LOADI_7(mrb_vm *vm, mrbc_value *regs, uint32_t ra)
{
  mrbc_release(&regs[ra]);
  regs[ra].tt = MRBC_TT_FIXNUM;
  regs[ra].i = 7;
  return 0;
}

//================================================================
/*!@brief
  Open the VM.

  @param vm     Pointer to mrbc_vm or NULL.
  @return	Pointer to mrbc_vm.
  @retval NULL	error.
*/
mrbc_vm *mrbc_vm_open( struct VM *vm_arg )
{
  mrbc_vm *vm;
  if( (vm = vm_arg) == NULL ) {
    // allocate memory.
    vm = (mrbc_vm *)mrbc_raw_alloc( sizeof(mrbc_vm) );
    if( vm == NULL ) return NULL;
  }

  // allocate vm id.
  int vm_id = 0;
  unsigned int i;
  for( i = 0; i < Num(free_vm_bitmap); i++ ) {
    int n = nlz32( ~free_vm_bitmap[i] );
    if( n < FREE_BITMAP_WIDTH ) {
      free_vm_bitmap[i] |= (1 << (FREE_BITMAP_WIDTH - n - 1));
      vm_id = i * FREE_BITMAP_WIDTH + n + 1;
      break;
    }
  }
  if( vm_id == 0 ) {
    if( vm_arg == NULL ) mrbc_raw_free(vm);
    return NULL;
  }

  // initialize attributes.
  memset(vm, 0, sizeof(mrbc_vm));	// caution: assume NULL is zero.
  if( vm_arg == NULL ) vm->flag_need_memfree = 1;
  vm->vm_id = vm_id;

  return vm;
}



//================================================================
/*!@brief
  Close the VM.

  @param  vm  Pointer to VM
*/
void mrbc_vm_close( struct VM *vm )
{
  // free vm id.
  unsigned int i = (vm->vm_id-1) / FREE_BITMAP_WIDTH;
  int n = (vm->vm_id-1) % FREE_BITMAP_WIDTH;
  assert( i < Num(free_vm_bitmap) );
  free_vm_bitmap[i] &= ~(1 << (FREE_BITMAP_WIDTH - n - 1));

  // free irep and vm
  if (vm->irep) mrbc_irep_free( vm->irep );
  if( vm->flag_need_memfree ) mrbc_raw_free(vm);
}



//================================================================
/*!@brief
  VM initializer.

  @param  vm  Pointer to VM
*/
void mrbc_vm_begin( struct VM *vm )
{
  vm->pc_irep = vm->irep;
  vm->pc = vm->pc_irep->code;
  vm->current_regs = vm->regs;
  memset(vm->regs, 0, sizeof(vm->regs));

  // clear regs
  int i;
  for( i = 1; i < MAX_REGS_SIZE; i++ ) {
    vm->regs[i].tt = MRBC_TT_NIL;
  }

  // set self to reg[0]
  vm->regs[0].tt = MRBC_TT_CLASS;
  vm->regs[0].cls = mrbc_class_object;

  vm->callinfo_tail = NULL;

  // target_class
  vm->target_class = mrbc_class_object;

  vm->error_code = 0;
  vm->flag_preemption = 0;
}


//================================================================
/*!@brief
  VM finalizer.

  @param  vm  Pointer to VM
*/
void mrbc_vm_end( struct VM *vm )
{
  mrbc_global_clear_vm_id();
  mrbc_free_all(vm);
}

static char const *to_opcode_str(uint8_t op) {
  switch (op) {
#define OPCODE(insn, ops) case OP_ ## insn: return "OP_" #insn;
#include "mruby/ops.h"
#undef OPCODE
  }
  return "unknown";
}

//================================================================
/*!@brief
  Fetch a bytecode and execute

  @param  vm    A pointer of VM.
  @retval 0  No error.
*/
int mrbc_vm_run( struct VM *vm )
{
  int ret = 0;
  uint32_t a;
  uint16_t b;
  uint8_t c;

  do {
    // regs
    mrbc_value *regs = vm->current_regs;
    uint8_t const insn = *(vm->pc);

    switch( insn ) {
#define pc (vm->pc)
#define EXTRACT_Z
#define EXTRACT_B , a
#define EXTRACT_BB , a, b
#define EXTRACT_BBB , a, b, c
#define EXTRACT_BS , a, b
#define EXTRACT_S , a
#define EXTRACT_W , a

#define CASE(op, func, args) \
      case OP_ ## op: \
        pc++; \
        FETCH_ ## args (); \
      L_ ## op ## _BODY: \
        ret = op_ ## func (vm, regs EXTRACT_ ## args); \
        break

    CASE(NOP, nop, Z);
    CASE(MOVE, move, BB);
    CASE(LOADL, loadl, BB);
    CASE(LOADI, loadi, BB);
    CASE(LOADSYM, loadsym, BB);
    CASE(LOADNIL, loadnil, B);
    CASE(LOADSELF, loadself, B);
    CASE(LOADT, loadt, B);
    CASE(LOADF, loadf, B);
    CASE(GETGV, getglobal, BB);
    CASE(SETGV, setglobal, BB);
    CASE(GETIV, getiv, BB);
    CASE(SETIV, setiv, BB);
    CASE(GETCONST, getconst, BB);
    CASE(SETCONST, setconst, BB);
    CASE(GETMCNST, getconst, BB);  // reuse
    CASE(GETUPVAR, getupvar, BBB);
    CASE(SETUPVAR, setupvar, BBB);
    CASE(JMP, jmp, S);
    CASE(JMPIF, jmpif, BS);
    CASE(JMPNOT, jmpnot, BS);
    CASE(SEND, send_noblk, BBB);
    CASE(SENDB, send_blk, BBB);  // reuse
    CASE(CALL, call, Z);
    CASE(SUPER, super, BB);
    CASE(ARGARY, argary, BS);
    CASE(ENTER, enter, W);
    CASE(RETURN, return, B);
    CASE(BLKPUSH, blkpush, BS);
    CASE(ADD, add, B);
    CASE(ADDI, addi, BB);
    CASE(SUB, sub, B);
    CASE(SUBI, subi, BB);
    CASE(MUL, mul, B);
    CASE(DIV, div, B);
    CASE(EQ, eq, B);
    CASE(LT, lt, B);
    CASE(LE, le, B);
    CASE(GT, gt, B);
    CASE(GE, ge, B);
    CASE(ARRAY, array, BB);
    CASE(STRING, string, BB);
    CASE(STRCAT, strcat, B);
    CASE(HASH, hash, BB);
    CASE(LAMBDA, lambda, BB);
    CASE(RANGE_INC, range_inc, B);
    CASE(RANGE_EXC, range_exc, B);
    CASE(CLASS, class, BB);
    CASE(EXEC, exec, BB);
    CASE(METHOD, method, BB);
    CASE(TCLASS, tclass, B);
    CASE(STOP, stop, Z);
    CASE(ABORT, abort, Z);
    CASE(DEF, DEF, BB);
    CASE(LOADI__1, LOADI__1, B);
    CASE(LOADI_0, LOADI_0, B);
    CASE(LOADI_1, LOADI_1, B);
    CASE(LOADI_2, LOADI_2, B);
    CASE(LOADI_3, LOADI_3, B);
    CASE(LOADI_4, LOADI_4, B);
    CASE(LOADI_5, LOADI_5, B);
    CASE(LOADI_6, LOADI_6, B);
    CASE(LOADI_7, LOADI_7, B);

    case OP_EXT1:
    L_EXT1_BODY: {
      uint8_t const insn = READ_B();
      switch (insn) {
#define OPCODE(insn,ops) case OP_ ## insn: FETCH_ ## ops ## _1(); goto L_ ## insn ## _BODY;
#include "mruby/ops.h"
#undef OPCODE
      }
      pc--;
      break;
    }
    case OP_EXT2:
    L_EXT2_BODY: {
      uint8_t const insn = READ_B();
      switch (insn) {
#define OPCODE(insn, ops) case OP_ ## insn: FETCH_ ## ops ## _2(); goto L_ ## insn ## _BODY;
#include "mruby/ops.h"
#undef OPCODE
      }
      pc--;
      break;
    }
    case OP_EXT3:
    L_EXT3_BODY: {
      uint8_t const insn = READ_B();
      switch (insn) {
#define OPCODE(insn, ops) case OP_ ## insn: FETCH_ ## ops ## _3(); goto L_ ## insn ## _BODY;
#include "mruby/ops.h"
#undef OPCODE
      }
      pc--;
      break;
    }

#define NOT_IMPL_OP(op, args) \
      case OP_ ## op: \
        pc++; \
        FETCH_ ## args (); \
      L_ ## op ## _BODY: \
        goto L_SKIP_OP

    NOT_IMPL_OP(LOADINEG, BB);
    NOT_IMPL_OP(ALIAS, BB);
    NOT_IMPL_OP(MODULE, BB);
    NOT_IMPL_OP(OCLASS, B);
    NOT_IMPL_OP(BLOCK, BB);
    NOT_IMPL_OP(HASHCAT, B);
    NOT_IMPL_OP(HASHADD, BB);
    NOT_IMPL_OP(INTERN, B);
    NOT_IMPL_OP(APOST, BBB);
    NOT_IMPL_OP(ASET, BBB);
    NOT_IMPL_OP(AREF, BBB);
    NOT_IMPL_OP(ARYDUP, B);
    NOT_IMPL_OP(ARYPUSH, B);
    NOT_IMPL_OP(ARYCAT, B);
    NOT_IMPL_OP(ARRAY2, BBB);
    NOT_IMPL_OP(BREAK, B);
    NOT_IMPL_OP(RETURN_BLK, B);
    NOT_IMPL_OP(KARG, BB);
    NOT_IMPL_OP(KEYEND, Z);
    NOT_IMPL_OP(KEY_P, BB);
    NOT_IMPL_OP(SENDVB, BB);
    NOT_IMPL_OP(SENDV, BB);
    NOT_IMPL_OP(RAISE, B);
    NOT_IMPL_OP(POPERR, B);
    NOT_IMPL_OP(EPOP, B);
    NOT_IMPL_OP(EPUSH, B);
    NOT_IMPL_OP(RESCUE, BB);
    NOT_IMPL_OP(EXCEPT, B);
    NOT_IMPL_OP(ONERR, S);
    NOT_IMPL_OP(JMPNIL, BS);
    NOT_IMPL_OP(SETMCNST, BB);
    NOT_IMPL_OP(SETCV, BB);
    NOT_IMPL_OP(GETCV, BB);
    NOT_IMPL_OP(SETSV, BB);
    NOT_IMPL_OP(GETSV, BB);
    NOT_IMPL_OP(UNDEF, B);
    NOT_IMPL_OP(SCLASS, B);
    NOT_IMPL_OP(DEBUG, BBB);
    NOT_IMPL_OP(ERR, B);

    default:
    L_SKIP_OP:
      console_printf("Skip OP=%02x\n", insn);
      break;
    }
  } while( !vm->flag_preemption );

  vm->flag_preemption = 0;

  return ret;
}

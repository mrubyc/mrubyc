/*! @file
  @brief
  mruby/c Proc class

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! proc constructor

  @param  vm		Pointer to VM.
  @param  irep		Pointer to IREP.
  @param  b_or_m	block or method flag.
  @return		mrbc_value of Proc object.
*/
mrbc_value mrbc_proc_new(struct VM *vm, void *irep, uint8_t b_or_m)
{
  mrbc_proc *proc = mrbc_alloc(vm, sizeof(mrbc_proc));

  memset(proc, 0, sizeof(mrbc_proc));
  MRBC_INIT_OBJECT_HEADER( proc, "PR" );
  proc->block_or_method = b_or_m;
  if( b_or_m == 'B' ) {
    if( mrbc_type(vm->cur_regs[0]) == MRBC_TT_PROC ) {
      proc->callinfo_self = vm->cur_regs[0].proc->callinfo_self;
      proc->self = vm->cur_regs[0].proc->self;
    } else {
      proc->callinfo_self = vm->callinfo_tail;
      proc->self = vm->cur_regs[0];
    }
    mrbc_incref(&proc->self);
  }
  proc->callinfo = vm->callinfo_tail;
  proc->irep = irep;

  /* Snapshot the current scope's regs so OP_GETUPVAR / OP_SETUPVAR keep
   * working after the parent function returns and its stack frame is
   * gone. Without this, the callinfo above would dangle because pop
   * frees it (use-after-free crash) or its slot gets recycled (silently
   * wrong upvar value).
   *
   * We capture all nregs regs of the enclosing irep so the chain walk
   * via reg[0] also works. */
  if( vm->cur_irep ) {
    int n = vm->cur_irep->nregs;
    proc->captured_regs_size = n;
    proc->captured_regs = mrbc_raw_alloc(n * sizeof(mrbc_value));
    for( int i = 0; i < n; i++ ) {
      proc->captured_regs[i] = vm->cur_regs[i];
      mrbc_incref(&proc->captured_regs[i]);
    }
  }

  return mrbc_immediate_value(MRBC_TT_PROC, .proc = proc);
}


//================================================================
/*! proc destructor

  @param  val	pointer to target value
*/
void mrbc_proc_delete(mrbc_value *val)
{
  mrbc_decref(&val->proc->self);
  if( val->proc->captured_regs ) {
    for( int i = 0; i < val->proc->captured_regs_size; i++ ) {
      mrbc_decref(&val->proc->captured_regs[i]);
    }
    mrbc_raw_free(val->proc->captured_regs);
  }
  mrbc_raw_free(val->proc);
}


#if defined(MRBC_ALLOC_VMID)
//================================================================
/*! clear vm_id

  @param  v		pointer to target.
*/
void mrbc_proc_clear_vm_id(mrbc_value *v)
{
  mrbc_set_vm_id( v->proc, 0 );
}
#endif



/***** Proc class ***********************************************************/
//================================================================
/*! (method) new
*/
static void c_proc_new(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[1]) != MRBC_TT_PROC ) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError),
               "tried to create Proc object without a block");
    return;
  }

  v[0] = v[1];
  mrbc_set_tt(&v[1], MRBC_TT_EMPTY);
}


//================================================================
/*! (method) call
*/
static void c_proc_call(mrbc_vm *vm, mrbc_value v[], int argc)
{
  assert( mrbc_type(v[0]) == MRBC_TT_PROC );

  /* NOTE: callinfo_self may dangle here -- the callinfo it pointed to
   * was freed when the proc's defining function returned. We MUST NOT
   * dereference it. Pushing with method_id=0 and leaving own_class as
   * the push_callinfo default (0) is safe; OP_GETCONST inside the
   * block will fall back to find_class_by_object(self) instead of
   * walking through callinfo->own_class. The cost is that constant
   * lookup inside a block runs against `self`'s class chain rather
   * than the defining method's class chain (rarely matters in
   * practice). */
  mrbc_callinfo *callinfo = mrbc_push_callinfo(vm, 0,
                                v - vm->cur_regs, argc);
  if( !callinfo ) return;
  callinfo->is_called_block = 1;

  // target irep
  vm->cur_irep = v[0].proc->irep;
  vm->inst = vm->cur_irep->inst;
  vm->cur_regs = v;
}


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Proc")
  FILE("_autogen_class_proc.h")

  METHOD( "new",	c_proc_new )
  METHOD( "call",	c_proc_call )
*/
#include "_autogen_class_proc.h"

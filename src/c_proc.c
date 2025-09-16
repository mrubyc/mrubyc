/*! @file
  @brief
  mruby/c Proc class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

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
  if( !proc ) goto RETURN;		// ENOMEM

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

 RETURN:
  return mrbc_immediate_value(MRBC_TT_PROC, .proc = proc);
}


//================================================================
/*! proc destructor

  @param  val	pointer to target value
*/
void mrbc_proc_delete(mrbc_value *val)
{
  mrbc_decref(&val->proc->self);
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
static void c_proc_new(struct VM *vm, mrbc_value v[], int argc)
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
static void c_proc_call(struct VM *vm, mrbc_value v[], int argc)
{
  assert( mrbc_type(v[0]) == MRBC_TT_PROC );

  mrbc_callinfo *callinfo_self = v[0].proc->callinfo_self;
  mrbc_callinfo *callinfo = mrbc_push_callinfo(vm,
				(callinfo_self ? callinfo_self->method_id : 0),
				v - vm->cur_regs, argc);
  if( !callinfo ) return;

  if( callinfo_self ) {
    callinfo->own_class = callinfo_self->own_class;
  }
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

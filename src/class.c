/*! @file
  @brief
  Class related functions.

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "vm_config.h"
#include <stdint.h>
#include <types.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <memory.h>

/***** Local headers ********************************************************/
#include "alloc.h"
#include "value.h"
#include "vm.h"
#include "class.h"
#include "symbol.h"
#include "keyvalue.h"
#include "global.h"
#include "console.h"
#include "load.h"
#include "c_string.h"
#include "c_array.h"
#include "c_hash.h"


/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/*! Builtin class table.

  @note must be same order as mrbc_vtype.
  @see mrbc_vtype in value.h
*/
mrbc_class * const mrbc_class_tbl[MRBC_TT_MAXVAL+1] = {
  0,				// MRBC_TT_EMPTY     = 0,
  MRBC_CLASS(NilClass),		// MRBC_TT_NIL	     = 1,
  MRBC_CLASS(FalseClass),	// MRBC_TT_FALSE     = 2,
  MRBC_CLASS(TrueClass),	// MRBC_TT_TRUE	     = 3,
  MRBC_CLASS(Integer),		// MRBC_TT_INTEGER   = 4,
#if MRBC_USE_FLOAT
  MRBC_CLASS(Float),		// MRBC_TT_FLOAT     = 5,
#endif /* MRBC_USE_FLOAT */
  MRBC_CLASS(Symbol),		// MRBC_TT_SYMBOL    = 6,
  0,				// MRBC_TT_CLASS     = 7,
  0,				// MRBC_TT_OBJECT    = 8,
  MRBC_CLASS(Proc),		// MRBC_TT_PROC	     = 9,
  MRBC_CLASS(Array),		// MRBC_TT_ARRAY     = 10,
  MRBC_CLASS(String),		// MRBC_TT_STRING    = 11,
  MRBC_CLASS(Range),		// MRBC_TT_RANGE     = 12,
  MRBC_CLASS(Hash),		// MRBC_TT_HASH	     = 13,
  0,				// MRBC_TT_EXCEPTION = 14,
};


/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/
//================================================================
/*! define class

  @param  vm		pointer to vm.
  @param  name		class name.
  @param  super		super class.
  @return		pointer to defined class.
*/
mrbc_class * mrbc_define_class(struct VM *vm, const char *name, mrbc_class *super)
{
  mrbc_sym sym_id = mrbc_str_to_symid(name);

  // already defined?
  mrbc_value *val = mrbc_get_const(sym_id);
  if( val ) {
    assert( mrbc_type(*val) == MRBC_TT_CLASS );
    return val->cls;
  }

  // create a new class.
  mrbc_class *cls = mrbc_raw_alloc_no_free( sizeof(mrbc_class) );
  if( !cls ) return cls;	// ENOMEM

  cls->sym_id = sym_id;
  cls->num_builtin_method = 0;
#ifdef MRBC_DEBUG
  cls->names = name;	// for debug; delete soon.
#endif
  cls->super = super ? super : mrbc_class_object;
  cls->method_link = 0;

  if( vm && mrbc_type(vm->cur_regs[0]) == MRBC_TT_CLASS ) {
    // For nested class
    //   so, not in TOPLEVEL, register to class constant
    assert(vm->target_class);
    mrbc_set_class_const(vm->target_class, sym_id,
			 &(mrbc_value){.tt = MRBC_TT_CLASS, .cls = cls});
  } else {
    // register to global constant.
    mrbc_set_const( sym_id, &(mrbc_value){.tt = MRBC_TT_CLASS, .cls = cls} );
  }

  return cls;
}


//================================================================
/*! define method.

  @param  vm		pointer to vm.
  @param  cls		pointer to class.
  @param  name		method name.
  @param  cfunc		pointer to function.
*/
void mrbc_define_method(struct VM *vm, mrbc_class *cls, const char *name, mrbc_func_t cfunc)
{
  if( cls == NULL ) cls = mrbc_class_object;	// set default to Object.

  mrbc_method *method = mrbc_raw_alloc_no_free( sizeof(mrbc_method) );
  if( !method ) return; // ENOMEM

  method->type = 'm';
  method->c_func = 1;
  method->sym_id = mrbc_str_to_symid( name );
  method->func = cfunc;
  method->next = cls->method_link;
  cls->method_link = method;
}


//================================================================
/*! instance constructor

  @param  vm    Pointer to VM.
  @param  cls	Pointer to Class (mrbc_class).
  @param  size	size of additional data.
  @return       mrbc_instance object.
*/
mrbc_value mrbc_instance_new(struct VM *vm, mrbc_class *cls, int size)
{
  mrbc_value v = {.tt = MRBC_TT_OBJECT};
  v.instance = mrbc_alloc(vm, sizeof(mrbc_instance) + size);
  if( v.instance == NULL ) return v;	// ENOMEM

  if( mrbc_kv_init_handle(vm, &v.instance->ivar, 0) != 0 ) {
    mrbc_raw_free(v.instance);
    v.instance = NULL;
    return v;
  }

  MRBC_INIT_OBJECT_HEADER( v.instance, "IN" );
  v.instance->cls = cls;

  return v;
}


//================================================================
/*! instance destructor

  @param  v	pointer to target value
*/
void mrbc_instance_delete(mrbc_value *v)
{
  mrbc_kv_delete_data( &v->instance->ivar );
  mrbc_raw_free( v->instance );
}


//================================================================
/*! instance variable setter

  @param  obj		pointer to target.
  @param  sym_id	key symbol ID.
  @param  v		pointer to value.
*/
void mrbc_instance_setiv(mrbc_value *obj, mrbc_sym sym_id, mrbc_value *v)
{
  mrbc_incref(v);
  mrbc_kv_set( &obj->instance->ivar, sym_id, v );
}


//================================================================
/*! instance variable getter

  @param  obj		pointer to target.
  @param  sym_id	key symbol ID.
  @return		value.
*/
mrbc_value mrbc_instance_getiv(mrbc_value *obj, mrbc_sym sym_id)
{
  mrbc_value *v = mrbc_kv_get( &obj->instance->ivar, sym_id );
  if( !v ) return mrbc_nil_value();

  mrbc_incref(v);
  return *v;
}


#if defined(MRBC_ALLOC_VMID)
//================================================================
/*! clear vm_id

  @param  v		pointer to target.
*/
void mrbc_instance_clear_vm_id(mrbc_value *v)
{
  mrbc_set_vm_id( v->instance, 0 );
  mrbc_kv_clear_vm_id( &v->instance->ivar );
}
#endif


//================================================================
/*! proc constructor

  @param  vm		Pointer to VM.
  @param  irep		Pointer to IREP.
  @return		mrbc_value of Proc object.
*/
mrbc_value mrbc_proc_new(struct VM *vm, void *irep)
{
  mrbc_value val = {.tt = MRBC_TT_PROC};

  val.proc = mrbc_alloc(vm, sizeof(mrbc_proc));
  if( !val.proc ) return val;	// ENOMEM

  MRBC_INIT_OBJECT_HEADER( val.proc, "PR" );
  val.proc->callinfo = vm->callinfo_tail;

  if( mrbc_type(vm->cur_regs[0]) == MRBC_TT_PROC ) {
    val.proc->callinfo_self = vm->cur_regs[0].proc->callinfo_self;
  } else {
    val.proc->callinfo_self = vm->callinfo_tail;
  }

  val.proc->irep = irep;

  return val;
}


//================================================================
/*! proc destructor

  @param  val	pointer to target value
*/
void mrbc_proc_delete(mrbc_value *val)
{
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



//================================================================
/*! Check the class is the class of object.

  @param  obj	target object
  @param  cls	class
  @return	result
*/
int mrbc_obj_is_kind_of( const mrbc_value *obj, const mrbc_class *cls )
{
  const mrbc_class *c = find_class_by_object( obj );
  while( c != NULL ) {
    if( c == cls ) return 1;
    c = c->super;
  }

  return 0;
}


//================================================================
/*! find method

  @param  r_method	pointer to mrbc_method to return values.
  @param  cls		search class.
  @param  sym_id	symbol id.
  @return		pointer to method or NULL.
*/
mrbc_method * mrbc_find_method( mrbc_method *r_method, mrbc_class *cls, mrbc_sym sym_id )
{
  do {
    mrbc_method *method;
    for( method = cls->method_link; method != 0; method = method->next ) {
      if( method->sym_id == sym_id ) {
	*r_method = *method;
	r_method->cls = cls;
	return r_method;
      }
    }

    struct RBuiltinClass *c = (struct RBuiltinClass *)cls;
    int right = c->num_builtin_method;
    if( right == 0 ) goto NEXT;
    int left = 0;

    while( left < right ) {
      int mid = (left + right) / 2;
      if( c->method_symbols[mid] < sym_id ) {
	left = mid + 1;
      } else {
	right = mid;
      }
    }

    if( c->method_symbols[right] == sym_id ) {
      *r_method = (mrbc_method){
	.type = 'm',
	.c_func = 2,
	.sym_id = sym_id,
	.func = c->method_functions[right],
	.cls = cls };
      return r_method;
    }

  NEXT:
    cls = cls->super;
  } while( cls != 0 );

  return 0;
}


//================================================================
/*! get class by name

  @param  name		class name.
  @return		pointer to class object.
*/
mrbc_class * mrbc_get_class_by_name( const char *name )
{
  mrbc_sym sym_id = mrbc_str_to_symid(name);
  mrbc_value *obj = mrbc_get_const(sym_id);

  if( obj == NULL ) return NULL;
  return (mrbc_type(*obj) == MRBC_TT_CLASS) ? obj->cls : NULL;
}


//================================================================
/*! (BETA) Call any method of the object, but written by C.

  @param  vm		pointer to vm.
  @param  v		see bellow example.
  @param  reg_ofs	see bellow example.
  @param  recv		pointer to receiver.
  @param  method_name	method name.
  @param  argc		num of params.

  @example
  // (Integer).to_s(16)
  static void c_integer_to_s(struct VM *vm, mrbc_value v[], int argc)
  {
    mrbc_value *recv = &v[1];
    mrbc_value arg1 = mrbc_integer_value(16);
    mrbc_value ret = mrbc_send( vm, v, argc, recv, "to_s", 1, &arg1 );
    SET_RETURN(ret);
  }
 */
mrbc_value mrbc_send( struct VM *vm, mrbc_value *v, int reg_ofs,
		     mrbc_value *recv, const char *method_name, int argc, ... )
{
  mrbc_method method;

  if( mrbc_find_method( &method, find_class_by_object(recv),
			mrbc_str_to_symid(method_name) ) == 0 ) {
    mrbc_printf("No method. vtype=%d method='%s'\n", mrbc_type(*recv), method_name );
    goto ERROR;
  }
  if( !method.c_func ) {
    mrbc_printf("Method %s needs to be C function.\n", method_name );
    goto ERROR;
  }

  // create call stack.
  mrbc_value *regs = v + reg_ofs + 2;
  mrbc_decref( &regs[0] );
  regs[0] = *recv;
  mrbc_incref(recv);

  va_list ap;
  va_start(ap, argc);
  int i;
  for( i = 1; i <= argc; i++ ) {
    mrbc_decref( &regs[i] );
    regs[i] = *va_arg(ap, mrbc_value *);
  }
  mrbc_decref( &regs[i] );
  regs[i] = mrbc_nil_value();
  va_end(ap);

  // call method.
  method.func(vm, regs, argc);
  mrbc_value ret = regs[0];

  for(; i >= 0; i-- ) {
    regs[i].tt = MRBC_TT_EMPTY;
  }

  return ret;

 ERROR:
  return mrbc_nil_value();
}


//================================================================
/*! (method) Ineffect operator / method
*/
void c_ineffect(struct VM *vm, mrbc_value v[], int argc)
{
  // nothing to do.
}


//================================================================
/*! Run mrblib, which is mruby bytecode
*/
static void mrbc_run_mrblib(const uint8_t bytecode[])
{
  // instead of mrbc_vm_open()
  mrbc_vm *vm = mrbc_alloc( 0, sizeof(mrbc_vm) );
  if( !vm ) return;	// ENOMEM
  memset(vm, 0, sizeof(mrbc_vm));

  mrbc_load_mrb(vm, bytecode);
  mrbc_vm_begin(vm);
  // VDP_drawText("in run mrblib2", 10, 7);
  mrbc_vm_run(vm);
  // VDP_drawText("in run mrblib3", 10, 7);
  mrbc_vm_end(vm);
  // VDP_drawText("in run mrblib4", 10, 8);

  // instead of mrbc_vm_close()
  mrbc_raw_free( vm->top_irep );	// free only top-level mrbc_irep.
					// (no need to free child ireps.)
  mrbc_raw_free( vm );
}


//================================================================
/*! initialize all classes.
 */
void mrbc_init_class(void)
{
  extern const uint8_t mrblib_bytecode[];
  void mrbc_init_class_math(void);
  mrbc_value cls = {.tt = MRBC_TT_CLASS};

  cls.cls = MRBC_CLASS(Object);
  mrbc_set_const( MRBC_SYM(Object), &cls );

  cls.cls = MRBC_CLASS(NilClass);
  mrbc_set_const( MRBC_SYM(NilClass), &cls );

  cls.cls = MRBC_CLASS(FalseClass);
  mrbc_set_const( MRBC_SYM(FalseClass), &cls );

  cls.cls = MRBC_CLASS(TrueClass);
  mrbc_set_const( MRBC_SYM(TrueClass), &cls );

  cls.cls = MRBC_CLASS(Integer);
  mrbc_set_const( MRBC_SYM(Integer), &cls );

#if MRBC_USE_FLOAT
  cls.cls = MRBC_CLASS(Float);
  mrbc_set_const( MRBC_SYM(Float), &cls );
#endif

  cls.cls = MRBC_CLASS(Symbol);
  mrbc_set_const( MRBC_SYM(Symbol), &cls );

  cls.cls = MRBC_CLASS(Proc);
  mrbc_set_const( MRBC_SYM(Proc), &cls );

  cls.cls = MRBC_CLASS(Array);
  mrbc_set_const( MRBC_SYM(Array), &cls );

#if MRBC_USE_STRING
  cls.cls = MRBC_CLASS(String);
  mrbc_set_const( MRBC_SYM(String), &cls );
#endif

  cls.cls = MRBC_CLASS(Range);
  mrbc_set_const( MRBC_SYM(Range), &cls );

  cls.cls = MRBC_CLASS(Hash);
  mrbc_set_const( MRBC_SYM(Hash), &cls );

#if MRBC_USE_MATH
  cls.cls = MRBC_CLASS(Math);
  mrbc_set_const( MRBC_SYM(Math), &cls );
  mrbc_init_class_math();
#endif

  cls.cls = MRBC_CLASS(Exception);
  mrbc_set_const( MRBC_SYM(Exception), &cls );

  cls.cls = MRBC_CLASS(NoMemoryError);
  mrbc_set_const( MRBC_SYM(NoMemoryError), &cls );

  cls.cls = MRBC_CLASS(StandardError);
  mrbc_set_const( MRBC_SYM(StandardError), &cls );

  cls.cls = MRBC_CLASS(ArgumentError);
  mrbc_set_const( MRBC_SYM(ArgumentError), &cls );

  cls.cls = MRBC_CLASS(IndexError);
  mrbc_set_const( MRBC_SYM(IndexError), &cls );

  cls.cls = MRBC_CLASS(NameError);
  mrbc_set_const( MRBC_SYM(NameError), &cls );

  cls.cls = MRBC_CLASS(NoMethodError);
  mrbc_set_const( MRBC_SYM(NoMethodError), &cls );

  cls.cls = MRBC_CLASS(RangeError);
  mrbc_set_const( MRBC_SYM(RangeError), &cls );

  cls.cls = MRBC_CLASS(RuntimeError);
  mrbc_set_const( MRBC_SYM(RuntimeError), &cls );

  cls.cls = MRBC_CLASS(TypeError);
  mrbc_set_const( MRBC_SYM(TypeError), &cls );

  cls.cls = MRBC_CLASS(ZeroDivisionError);
  mrbc_set_const( MRBC_SYM(ZeroDivisionError), &cls );

  // VDP_drawText("before run mrblib.", 10, 5);
  mrbc_run_mrblib(mrblib_bytecode);
}

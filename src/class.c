/*! @file
  @brief
  Class related functions.

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
#include <stdint.h>
#include <string.h>
#include <assert.h>
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

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
  0,                            // MRBC_TT_EMPTY     = 0,
  MRBC_CLASS(NilClass),         // MRBC_TT_NIL       = 1,
  MRBC_CLASS(FalseClass),       // MRBC_TT_FALSE     = 2,
  MRBC_CLASS(TrueClass),        // MRBC_TT_TRUE      = 3,
  MRBC_CLASS(Integer),          // MRBC_TT_INTEGER   = 4,
  MRBC_CLASS(Float),            // MRBC_TT_FLOAT     = 5,
  MRBC_CLASS(Symbol),           // MRBC_TT_SYMBOL    = 6,
  0,                            // MRBC_TT_CLASS     = 7,
  0,                            // MRBC_TT_MODULE    = 8,
  0,                            // MRBC_TT_OBJECT    = 9,
  MRBC_CLASS(Proc),             // MRBC_TT_PROC      = 10,
  MRBC_CLASS(Array),            // MRBC_TT_ARRAY     = 11,
  MRBC_CLASS(String),           // MRBC_TT_STRING    = 12,
  MRBC_CLASS(Range),            // MRBC_TT_RANGE     = 13,
  MRBC_CLASS(Hash),             // MRBC_TT_HASH      = 14,
  0,                            // MRBC_TT_EXCEPTION = 15,
};


/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
//================================================================
/*! (internal use) traverse class tree.

  @param  cls		target class
  @param  nest_buf	nest buffer
  @param  nest_idx	nest buffer index
  @return mrbc_class *	next target class or NULL
*/
mrbc_class * mrbc_traverse_class_tree( mrbc_class *cls, mrbc_class *nest_buf[], int *nest_idx )
{
  cls = cls->super;

  if( cls == 0 ) {
    if( *nest_idx == 0 ) return 0;	// does not have super class.
    cls = nest_buf[--(*nest_idx)];	// rewind to the saved point.
    cls = cls->super;
  }

  // is the next module alias?
  if( cls->flag_alias ) {
    if( cls->super ) {
      // save the branch point to nest_buf.
      if( *nest_idx >= MRBC_TRAVERSE_NEST_LEVEL ) {
        mrbc_printf("Warning: Module nest exceeds upper limit.\n");
      } else {
        nest_buf[(*nest_idx)++] = cls;
      }
    }

    cls = cls->aliased;
  }

  return cls;
}


//================================================================
/*! (internal use) traverse class tree. skip that class.

  @param  nest_buf	nest buffer
  @param  nest_idx	nest buffer index
  @return mrbc_class *	previous target class or NULL
*/
mrbc_class * mrbc_traverse_class_tree_skip( mrbc_class *nest_buf[], int *nest_idx )
{
  if( *nest_idx == 0 ) return 0;	// does not have super class.
  return nest_buf[--(*nest_idx)];	// rewind to the saved point.
}


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
  if( sym_id < 0 ) {
    mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow MAX_SYMBOLS_COUNT");
    return 0;
  }

  // already defined?
  const mrbc_value *val = mrbc_get_const(sym_id);
  if( val ) {
    if( mrbc_type(*val) != MRBC_TT_CLASS ) {
      mrbc_raisef(vm, MRBC_CLASS(TypeError), "%s is not a class", name);
    }
    return val->cls;
  }

  // create a new class.
  mrbc_class *cls = mrbc_raw_alloc_no_free( sizeof(mrbc_class) );

  *cls = (mrbc_class){
#if defined(MRBC_DEBUG)
    .obj_mark_ = "CL",
    .name = name,
#endif
    .sym_id = sym_id,
    .super = super ? super : MRBC_CLASS(Object),
  };

  // register to global constant
  mrbc_set_const( sym_id, &mrbc_immediate_value(MRBC_TT_CLASS, .cls = cls) );

  return cls;
}


//================================================================
/*! define nested class

  @param  vm		pointer to vm.
  @param  outer		outer class
  @param  name		class name.
  @param  super		super class.
  @return		pointer to defined class.
*/
mrbc_class * mrbc_define_class_under(struct VM *vm, const mrbc_class *outer, const char *name, mrbc_class *super)
{
  mrbc_sym sym_id = mrbc_str_to_symid(name);
  if( sym_id < 0 ) {
    mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow MAX_SYMBOLS_COUNT");
    return 0;
  }

  // already defined?
  const mrbc_value *val = mrbc_get_class_const( outer, sym_id );
  if( val ) {
    if( mrbc_type(*val) != MRBC_TT_CLASS ) {
      mrbc_raisef(vm, MRBC_CLASS(TypeError), "%s is not a class", name);
      return 0;
    }
    return val->cls;
  }

  // create a new nested class.
  mrbc_class *cls = mrbc_raw_alloc_no_free( sizeof(mrbc_class) );

  char buf[sizeof(mrbc_sym)*4+1];
  make_nested_symbol_s( buf, outer->sym_id, sym_id );

  *cls = (mrbc_class){
#if defined(MRBC_DEBUG)
    .obj_mark_ = "CL",
    .name = name,
#endif
    .sym_id = mrbc_symbol( mrbc_symbol_new( vm, buf )),
    .super = super ? super : MRBC_CLASS(Object),
  };

  // register to global constant
  mrbc_set_class_const( outer, sym_id, &mrbc_immediate_value(MRBC_TT_CLASS, .cls = cls) );

  return cls;
}


//================================================================
/*! define module

  @param  vm		pointer to vm.
  @param  name		module name.
  @return		pointer to defined module.
*/
mrbc_class * mrbc_define_module(struct VM *vm, const char *name)
{
  mrbc_sym sym_id = mrbc_str_to_symid(name);
  if( sym_id < 0 ) {
    mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow MAX_SYMBOLS_COUNT");
    return 0;
  }

  // already defined?
  const mrbc_value *val = mrbc_get_const(sym_id);
  if( val ) {
    if( mrbc_type(*val) != MRBC_TT_MODULE ) {
      mrbc_raisef(vm, MRBC_CLASS(TypeError), "%s is not a module", name);
    }
    return val->cls;
  }

  // create a new module.
  mrbc_class *cls = mrbc_raw_alloc_no_free( sizeof(mrbc_class) );

  *cls = (mrbc_class){
#if defined(MRBC_DEBUG)
    .obj_mark_ = "MO",
    .name = name,
#endif
    .sym_id = sym_id,
    .flag_module = 1,
    .super = 0,
  };

  // register to global constant
  mrbc_set_const( sym_id, &mrbc_immediate_value(MRBC_TT_MODULE, .cls = cls) );

  return cls;
}


//================================================================
/*! define nested module

  @param  vm		pointer to vm.
  @param  outer		outer module
  @param  name		module name.
  @return		pointer to defined module.
*/
mrbc_class * mrbc_define_module_under(struct VM *vm, const mrbc_class *outer, const char *name)
{
  mrbc_sym sym_id = mrbc_str_to_symid(name);
  if( sym_id < 0 ) {
    mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow MAX_SYMBOLS_COUNT");
    return 0;
  }

  // already defined?
  const mrbc_value *val = mrbc_get_class_const( outer, sym_id );
  if( val ) {
    if( mrbc_type(*val) != MRBC_TT_MODULE ) {
      mrbc_raisef(vm, MRBC_CLASS(TypeError), "%s is not a module", name);
      return 0;
    }
    return val->cls;
  }

  // create a new nested module
  mrbc_class *cls = mrbc_raw_alloc_no_free( sizeof(mrbc_class) );

  char buf[sizeof(mrbc_sym)*4+1];
  make_nested_symbol_s( buf, outer->sym_id, sym_id );

  *cls = (mrbc_class){
#if defined(MRBC_DEBUG)
    .obj_mark_ = "MO",
    .name = name,
#endif
    .sym_id = mrbc_symbol( mrbc_symbol_new( vm, buf )),
    .flag_module = 1,
    .super = 0,
  };

  // register to global constant
  mrbc_set_class_const( outer, sym_id, &mrbc_immediate_value(MRBC_TT_MODULE, .cls = cls) );

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
  if( cls == NULL ) cls = MRBC_CLASS(Object);	// set default to Object.

  mrbc_method *method = mrbc_raw_alloc_no_free( sizeof(mrbc_method) );

  method->type = 'm';
  method->c_func = 1;
  method->sym_id = mrbc_str_to_symid( name );
  if( method->sym_id < 0 ) {
    mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow MAX_SYMBOLS_COUNT");
  }
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
  mrbc_value v = mrbc_immediate_value(MRBC_TT_OBJECT);

  v.instance = mrbc_alloc(vm, sizeof(mrbc_instance) + size);
  mrbc_kv_init_handle(vm, &v.instance->ivar, 0);
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
  assert( mrbc_type(*v) == MRBC_TT_OBJECT );
  mrbc_class *cls = v->instance->cls;

  if( !cls->flag_builtin && cls->destructor ) cls->destructor( v );

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

  @param  obj		target object.
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
/*! Check the class is the class of object.

  @param  obj	target object
  @param  tcls	target class
  @return	result
*/
int mrbc_obj_is_kind_of( const mrbc_value *obj, const mrbc_class *tcls )
{
  mrbc_class *cls = find_class_by_object( obj );
  mrbc_class *nest_buf[MRBC_TRAVERSE_NEST_LEVEL];
  int nest_idx = 0;

  while( cls != tcls ) {
    cls = mrbc_traverse_class_tree( cls, nest_buf, &nest_idx );
    if( ! cls ) return 0;
  }

  return 1;
}


//================================================================
/*! find method

  @param  r_method	pointer to mrbc_method to return values.
  @param  cls		search class or module.
  @param  sym_id	search symbol id.
  @return		pointer to method or NULL.
*/
mrbc_method * mrbc_find_method( mrbc_method *r_method, mrbc_class *cls, mrbc_sym sym_id )
{
  mrbc_class *nest_buf[MRBC_TRAVERSE_NEST_LEVEL];
  int nest_idx = 0;
  int flag_module = cls->flag_module;

  while( 1 ) {
    mrbc_method *method;
    /* When flag_alias==1, the union holds 'aliased' (a class pointer) instead
       of method_link. When flag_builtin==1 and num_builtin_method==0
       (RBuiltinNoMethodClass), there is no method_link field.
       Skip the dynamic method search in those cases. */
    if( !cls->flag_alias && (!cls->flag_builtin || cls->num_builtin_method > 0) ) {
      for( method = cls->method_link; method != 0; method = method->next ) {
        if( method->sym_id == sym_id ) {
          *r_method = *method;
          r_method->cls = cls;
          return r_method;
        }
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
      r_method->type = 'm';
      r_method->c_func = 2;
      r_method->sym_id = sym_id;
      r_method->func = c->method_functions[right];
      r_method->cls = cls;
      return r_method;
    }

  NEXT:
    cls = mrbc_traverse_class_tree( cls, nest_buf, &nest_idx );
    if( cls == 0 ) {
      if( !flag_module ) break;

      cls = MRBC_CLASS(Object);
      flag_module = 0;
    }
  }  // loop next.

  return 0;
}


//================================================================
/*! get class by name

  @param  name		class name.
  @return		pointer to class object.
*/
mrbc_class * mrbc_get_class_by_name( const char *name )
{
  mrbc_sym sym_id = mrbc_search_symid(name);
  if( sym_id < 0 ) return NULL;

  mrbc_value *obj = mrbc_get_const(sym_id);
  if( obj == NULL ) return NULL;

  if( mrbc_type(*obj) == MRBC_TT_CLASS ||
      mrbc_type(*obj) == MRBC_TT_MODULE ) return obj->cls;

  return NULL;
}


//================================================================
/*! (BETA) Call any method of the object, but written by C.

  @param  vm		pointer to vm.
  @param  v		see below example.
  @param  argc		see below example.
  @param  recv		pointer to receiver.
  @param  method_name	method name.
  @param  n_params	num of params.

<b>Examples</b>
@code
  // (Integer).to_s(16)
  static void c_integer_to_s(struct VM *vm, mrbc_value v[], int argc)
  {
    mrbc_value *recv = &v[1];	// expect Integer.
    mrbc_value arg1 = mrbc_integer_value(16);
    mrbc_value ret = mrbc_send( vm, v, argc, recv, "to_s", 1, &arg1 );
    SET_RETURN(ret);
  }
@endcode
*/
mrbc_value mrbc_send( struct VM *vm, mrbc_value *v, int argc,
        mrbc_value *recv, const char *method_name, int n_params, ... )
{
  mrbc_method method;
  mrbc_class *cls = find_class_by_object(recv);

  if( mrbc_find_method( &method, cls, mrbc_str_to_symid(method_name)) == 0 ) {
    mrbc_raisef(vm, MRBC_CLASS(NoMethodError), "undefined method '%s' for %s",
                method_name, mrbc_symid_to_str(cls->sym_id) );
    goto ERROR;
  }
  if( !method.c_func ) {
    mrbc_raisef(vm, MRBC_CLASS(NotImplementedError),
                "Method needs to be C function. '%s' for %s",
                method_name, mrbc_symid_to_str(cls->sym_id) );
    goto ERROR;
  }

  // create call stack.
  mrbc_value *regs = v + argc + 2;
  mrbc_decref( &regs[0] );
  regs[0] = *recv;
  mrbc_incref(recv);

  va_list ap;
  va_start(ap, n_params);
  int i;
  for( i = 1; i <= n_params; i++ ) {
    mrbc_decref( &regs[i] );
    regs[i] = *va_arg(ap, mrbc_value *);
  }
  mrbc_decref( &regs[i] );
  regs[i] = mrbc_nil_value();
  va_end(ap);

  // call method.
  method.func(vm, regs, n_params);
  mrbc_value ret = regs[0];

  for(; i >= 0; i-- ) {
    mrbc_set_tt(&regs[i], MRBC_TT_EMPTY);
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

  @param  bytecode	bytecode (.mrb file)
  @return		dummy yet.
*/
int mrbc_run_mrblib(const void *bytecode)
{
  // instead of mrbc_vm_open()
  mrbc_vm *vm = mrbc_vm_new( MAX_REGS_SIZE );

  if( mrbc_load_mrb(vm, bytecode) ) {
    mrbc_print_vm_exception(vm);
    return 2;
  }

  int ret;

  mrbc_vm_begin(vm);
  do {
    ret = mrbc_vm_run(vm);
  } while( ret == 0 );
  mrbc_vm_end(vm);
  mrbc_vm_close(vm);

  return ret;
}

#define MRBC_DEFINE_BUILTIN_CLASS_TABLE
#include "_autogen_builtin_class.h"
#undef MRBC_DEFINE_BUILTIN_CLASS_TABLE

//================================================================
/*! initialize all classes.
 */
void mrbc_init_class(void)
{
  // initialize builtin class.
  mrbc_value vcls;

  for( int i = 0; i < sizeof(MRBC_BuiltinClass)/sizeof(struct MRBC_BuiltinClass); i++ ) {
    mrbc_class *cls = MRBC_BuiltinClass[i].cls;

    cls->super = MRBC_BuiltinClass[i].super;
    if( !cls->flag_nomethod ) cls->method_link = 0;
    mrbc_set_tt( &vcls, cls->flag_module ? MRBC_TT_MODULE : MRBC_TT_CLASS );
    vcls.cls = cls;
    mrbc_set_const( cls->sym_id, &vcls );
  }

#if MRBC_USE_MATH
  mrbc_init_module_math();
#endif

  extern const uint8_t mrblib_bytecode[];
  mrbc_run_mrblib(mrblib_bytecode);
}

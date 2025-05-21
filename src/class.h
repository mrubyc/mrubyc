/*! @file
  @brief
  Class related functions.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_CLASS_H_
#define MRBC_SRC_CLASS_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdint.h>
//@endcond

/***** Local headers ********************************************************/
#include "value.h"
#include "keyvalue.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** Constant values ******************************************************/
#define MRBC_TRAVERSE_NEST_LEVEL 3


/***** Macros ***************************************************************/
/*!
  Get a built-in class (pointer)

  @param  cls	Class name. (e.g. Array)
  @return	Pointer to the class.

  @details
  Example
  @code
    mrbc_class *cls = MRBC_CLASS(Array);	// get a Array class
    mrbc_class *cls = MRBC_CLASS(String);	// get a String class
  @endcode
*/
#define MRBC_CLASS(cls)	((mrbc_class *)(&mrbc_class_##cls))

/*!
  Get a pointer to mrbc_instance->data converted to specified type.

  @param  v	Pointer to mrbc_instance.
  @param  t	Type of return pointer.

  @details
  Example
  @code
    // store a int value.
    *MRBC_INSTANCE_DATA_PTR(v, int) = n;

    // get
    int n = *MRBC_INSTANCE_DATA_PTR(v, int);

    // store a pointer to statically allocated memory.
    *MRBC_INSTANCE_DATA_PTR(v, struct STATIC_STRUCT *) = STATIC_STRUCT;

    // get
    struct STATIC_STRUCT *p = *MRBC_INSTANCE_DATA_PTR(v, struct STATIC_STRUCT *);
  @endcode
*/
#define MRBC_INSTANCE_DATA_PTR(v, t) ((t *)((v)->instance->data))


/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  Class object.
*/
typedef struct RClass {
  mrbc_sym sym_id;		 //!< class name's symbol ID
  unsigned int flag_builtin : 1; //!< is built-in class? (= 0)
  unsigned int flag_module : 1;  //!< is module?
  unsigned int flag_alias : 1;   //!< is module alias?
  uint8_t num_builtin_method;	 //!< num of built-in method.
  struct RClass *super;		 //!< pointer to super class.
  union {
    struct RMethod *method_link; //!< pointer to method link.
    struct RClass *aliased;      //!< aliased class or module.
  };
#if defined(MRBC_DEBUG)
  const char *name;
#endif

  void (*destructor)( mrbc_value * );	//!< specify a destructor if need.
} mrbc_class;
typedef struct RClass mrb_class;

//================================================================
/*!@brief
  Built-in class object.

  @extends RClass
*/
struct RBuiltinClass {
  mrbc_sym sym_id;		 //!< class name's symbol ID
  unsigned int flag_builtin : 1; //!< is built-in class? (= 1)
  unsigned int flag_module : 1;  //!< is module?
  unsigned int flag_alias : 1;   //!< is alias class?
  uint8_t num_builtin_method;	 //!< num of built-in method.
  struct RClass *super;		 //!< pointer to super class.
  union {
    struct RMethod *method_link; //!< pointer to method link.
    struct RClass *aliased;      //!< aliased class or module.
  };
#if defined(MRBC_DEBUG)
  const char *name;
#endif

  const mrbc_sym *method_symbols;	//!< built-in method sym-id table.
  const mrbc_func_t *method_functions;	//!< built-in method function table.
};

//================================================================
/*!@brief
  Built-in No method class object.

  @extends RBuiltinClass
*/
struct RBuiltinNoMethodClass {
  mrbc_sym sym_id;		 //!< class name's symbol ID
  unsigned int flag_builtin : 1; //!< is built-in class? (= 1)
  unsigned int flag_module : 1;  //!< is module?
  unsigned int flag_alias : 1;   //!< is alias class?
  uint8_t num_builtin_method;	 //!< num of built-in method.
  struct RClass *super;		 //!< pointer to super class.
  union {
    struct RMethod *method_link; //!< pointer to method link.
    struct RClass *aliased;      //!< aliased class or module.
  };
#if defined(MRBC_DEBUG)
  const char *name;
#endif
};

//================================================================
/*!@brief
  Instance object.

  @extends RBasic
*/
typedef struct RInstance {
  MRBC_OBJECT_HEADER;

  struct RClass *cls;		//!< pointer to class of this object.
  struct RKeyValueHandle ivar;	//!< instance variable.
  uint8_t data[];		//!< extended data

} mrbc_instance;
typedef struct RInstance mrb_instance;


//================================================================
/*!@brief
  Method management structure.
*/
typedef struct RMethod {
  uint8_t type;		//!< M:OP_DEF or OP_ALIAS, m:mrblib or define_method()
  uint8_t c_func;	//!< 0:IREP, 1:C Func, 2:C Func (built-in)
  mrbc_sym sym_id;	//!< function names symbol ID
  union {
    struct IREP *irep;	//!< to IREP for ruby proc.
    mrbc_func_t func;	//!< to C function.
  };
  union {
    struct RMethod *next;	//!< link to next method.
    struct RClass *cls;		//!< return value for mrbc_find_method.
  };
} mrbc_method;


/***** Global variables *****************************************************/
extern struct RClass * const mrbc_class_tbl[];
#include "_autogen_builtin_class.h"

// for old version compatibility.
#define mrbc_class_object ((struct RClass*)(&mrbc_class_Object))


/***** Function prototypes **************************************************/
//@cond
mrbc_class *mrbc_traverse_class_tree(mrbc_class *cls, mrbc_class *nest_buf[], int *nest_idx);
mrbc_class *mrbc_traverse_class_tree_skip(mrbc_class *nest_buf[], int *nest_idx);
mrbc_class *mrbc_define_class(struct VM *vm, const char *name, mrbc_class *super);
mrbc_class *mrbc_define_class_under(struct VM *vm, const mrbc_class *outer, const char *name, mrbc_class *super);
mrbc_class *mrbc_define_module(struct VM *vm, const char *name);
mrbc_class *mrbc_define_module_under(struct VM *vm, const mrbc_class *outer, const char *name);
void mrbc_define_method(struct VM *vm, mrbc_class *cls, const char *name, mrbc_func_t cfunc);
mrbc_value mrbc_instance_new(struct VM *vm, mrbc_class *cls, int size);
void mrbc_instance_delete(mrbc_value *v);
void mrbc_instance_setiv(mrbc_value *obj, mrbc_sym sym_id, mrbc_value *v);
mrbc_value mrbc_instance_getiv(mrbc_value *obj, mrbc_sym sym_id);
void mrbc_instance_clear_vm_id(mrbc_value *v);
int mrbc_obj_is_kind_of(const mrbc_value *obj, const mrbc_class *tcls);
mrbc_method *mrbc_find_method(mrbc_method *r_method, mrbc_class *cls, mrbc_sym sym_id);
mrbc_class *mrbc_get_class_by_name(const char *name);
mrbc_value mrbc_send(struct VM *vm, mrbc_value *v, int argc, mrbc_value *recv, const char *method_name, int n_params, ...);
void c_ineffect(struct VM *vm, mrbc_value v[], int argc);
int mrbc_run_mrblib(const void *bytecode);
void mrbc_init_class(void);
//@endcond


/***** Inline functions *****************************************************/

//================================================================
/*! find class by object

  @param  obj	pointer to object
  @return	pointer to mrbc_class
*/
static inline mrbc_class *find_class_by_object(const mrbc_value *obj)
{
  assert( mrbc_type(*obj) >= 0 );
  assert( mrbc_type(*obj) <= MRBC_TT_MAXVAL );

  mrbc_class *cls = mrbc_class_tbl[ mrbc_type(*obj) ];
  if( !cls ) {
    switch( mrbc_type(*obj) ) {
    case MRBC_TT_CLASS:		// fall through.
    case MRBC_TT_MODULE:	cls = obj->cls;			break;
    case MRBC_TT_OBJECT:	cls = obj->instance->cls;	break;
    case MRBC_TT_EXCEPTION:	cls = obj->exception->cls;	break;
    default:
      assert(!"Invalid value type.");
    }
  }

  return cls;
}


//================================================================
/*! Define the destructor

  @param  cls		class.
  @param  destructor	destructor.
  @details
  Define the destructor.
  This function can only be defined for destruction of mrbc_instance.
*/
static inline void mrbc_define_destructor( mrbc_class *cls, void (*destructor)(mrbc_value *) )
{
  assert( cls->flag_builtin == 0 );
  assert( cls->flag_module == 0 );

  cls->destructor = destructor;
}


//================================================================
/*! instance variable getter

  @param  obj		target object.
  @param  sym_id	key symbol ID.
  @return		pointer to value or NULL.
*/
static inline mrbc_value * mrbc_instance_getiv_p(mrbc_value *obj, mrbc_sym sym_id)
{
  return mrbc_kv_get( &obj->instance->ivar, sym_id );
}


#ifdef __cplusplus
}
#endif
#endif

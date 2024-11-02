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
/***** Macros ***************************************************************/
#define MRBC_CLASS(cls)	((mrbc_class *)(&mrbc_class_##cls))


/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  Class object.
*/
typedef struct RClass {
  mrbc_sym sym_id;		//!< class name's symbol ID
  unsigned int flag_builtin : 1;//!< is built-in class?
  unsigned int flag_module : 1; //!< is module?
  unsigned int flag_alias : 1;  //!< is alias class?
  uint8_t num_builtin_method;	//!< num of built-in method.
  struct RClass *super;		//!< pointer to super class.
  union {
    struct RMethod *method_link;//!< pointer to method link.
    struct RClass *aliased;     //!< aliased class or module.
  };
#if defined(MRBC_DEBUG)
  const char *name;
#endif
} mrbc_class;
typedef struct RClass mrb_class;

//================================================================
/*!@brief
  Built-in class object.

  @extends RClass
*/
struct RBuiltinClass {
  mrbc_sym sym_id;		//!< class name's symbol ID
  unsigned int flag_builtin : 1;//!< is built-in class?
  unsigned int flag_module : 1; //!< is module?
  unsigned int flag_alias : 1;  //!< is alias class?
  uint8_t num_builtin_method;	//!< num of built-in method.
  struct RClass *super;		//!< pointer to super class.
  union {
    struct RMethod *method_link;//!< pointer to method link.
    struct RClass *aliased;     //!< aliased class or module.
  };
#if defined(MRBC_DEBUG)
  const char *name;
#endif

  const mrbc_sym *method_symbols;	//!< built-in method sym-id table.
  const mrbc_func_t *method_functions;	//!< built-in method function table.
};


//================================================================
/*!@brief
  Instance object.

  @extends RBasic
*/
typedef struct RInstance {
  MRBC_OBJECT_HEADER;

  struct RClass *cls;
  struct RKeyValueHandle ivar;
  uint8_t data[];

} mrbc_instance;
typedef struct RInstance mrb_instance;


//================================================================
/*!@brief
  Proc object.

  @extends RBasic
*/
typedef struct RProc {
  MRBC_OBJECT_HEADER;

  uint8_t block_or_method;
  struct CALLINFO *callinfo;
  struct CALLINFO *callinfo_self;
  struct IREP *irep;
  mrbc_value self;
  mrbc_value ret_val;

} mrbc_proc;
typedef struct RProc mrb_proc;


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
extern struct RBuiltinClass mrbc_class_Object;
extern struct RBuiltinClass mrbc_class_NilClass;
extern struct RBuiltinClass mrbc_class_FalseClass;
extern struct RBuiltinClass mrbc_class_TrueClass;
extern struct RBuiltinClass mrbc_class_Integer;
extern struct RBuiltinClass mrbc_class_Float;
extern struct RBuiltinClass mrbc_class_Symbol;
extern struct RBuiltinClass mrbc_class_Proc;
extern struct RBuiltinClass mrbc_class_Array;
extern struct RBuiltinClass mrbc_class_String;
extern struct RBuiltinClass mrbc_class_Range;
extern struct RBuiltinClass mrbc_class_Hash;
extern struct RBuiltinClass mrbc_class_Math;
extern struct RBuiltinClass mrbc_class_Exception;
extern struct RClass mrbc_class_NoMemoryError;
extern struct RClass mrbc_class_NotImplementedError;
extern struct RClass mrbc_class_StandardError;
extern struct RClass mrbc_class_ArgumentError;
extern struct RClass mrbc_class_IndexError;
extern struct RClass mrbc_class_IOError;
extern struct RClass mrbc_class_NameError;
extern struct RClass mrbc_class_NoMethodError;
extern struct RClass mrbc_class_RangeError;
extern struct RClass mrbc_class_RuntimeError;
extern struct RClass mrbc_class_TypeError;
extern struct RClass mrbc_class_ZeroDivisionError;

// for old version compatibility.
#define mrbc_class_object ((struct RClass*)(&mrbc_class_Object))


/***** Function prototypes **************************************************/
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
mrbc_value mrbc_proc_new(struct VM *vm, void *irep, uint8_t b_or_m);
void mrbc_proc_delete(mrbc_value *val);
void mrbc_proc_clear_vm_id(mrbc_value *v);
int mrbc_obj_is_kind_of(const mrbc_value *obj, const mrbc_class *cls);
mrbc_method *mrbc_find_method(mrbc_method *r_method, mrbc_class *cls, mrbc_sym sym_id);
mrbc_class *mrbc_get_class_by_name(const char *name);
mrbc_value mrbc_send(struct VM *vm, mrbc_value *v, int reg_ofs, mrbc_value *recv, const char *method_name, int argc, ...);
void c_ineffect(struct VM *vm, mrbc_value v[], int argc);
int mrbc_run_mrblib(const void *bytecode);
void mrbc_init_class(void);


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


#ifdef __cplusplus
}
#endif
#endif

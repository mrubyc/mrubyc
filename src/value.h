/*! @file
  @brief
  mruby/c value definitions

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_VALUE_H_
#define MRBC_SRC_VALUE_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include <stdint.h>
#include <assert.h>
#include "vm_config.h"
//@endcond

/***** Local headers ********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***** Constant values ******************************************************/
/***** Typedefs *************************************************************/
// pre define of some struct
struct VM;
struct RObject;

// mrbc types
#if defined(MRBC_INT16)
typedef int16_t mrbc_int_t;
typedef uint16_t mrbc_uint_t;
#elif defined(MRBC_INT64)
typedef int64_t mrbc_int_t;
typedef uint64_t mrbc_uint_t;
#else
typedef int32_t mrbc_int_t;
typedef uint32_t mrbc_uint_t;
#endif
typedef mrbc_int_t mrb_int;

#if MRBC_USE_FLOAT == 1
typedef float mrbc_float_t;
#elif MRBC_USE_FLOAT == 2
typedef double mrbc_float_t;
#endif
#if MRBC_USE_FLOAT != 0
typedef mrbc_float_t mrb_float;
#endif

typedef int16_t mrbc_sym;	//!< mruby/c symbol ID
typedef void (*mrbc_func_t)(struct VM *vm, struct RObject *v, int argc);


//================================================================
/*! value type in mrbc_value.
*/
typedef enum {
  /* (note) Must be same order as mrbc_class_tbl[], mrbc_delfunc[]. */

  /* internal use */
  MRBC_TT_JMPUW         = -5,  // use in OP_JMPUW...
  MRBC_TT_BREAK         = -4,
  MRBC_TT_RETURN_BLK    = -3,
  MRBC_TT_RETURN        = -2,
  MRBC_TT_HANDLE        = -1,

  /* primitive */
  MRBC_TT_EMPTY     = 0,
  MRBC_TT_NIL       = 1,        //!< NilClass
  MRBC_TT_FALSE     = 2,        //!< FalseClass
  // (note) true/false threshold. see op_jmpif

  MRBC_TT_TRUE      = 3,        //!< TrueClass
  MRBC_TT_INTEGER   = 4,        //!< Integer
  MRBC_TT_FIXNUM    = 4,
  MRBC_TT_FLOAT     = 5,        //!< Float
  MRBC_TT_SYMBOL    = 6,        //!< Symbol
  MRBC_TT_CLASS     = 7,        //!< Class
  MRBC_TT_MODULE    = 8,        //!< Module
  // (note) inc/dec ref threshold.

  /* non-primitive */
  MRBC_TT_OBJECT    = 9,        //!< General instance
  MRBC_TT_PROC      = 10,       //!< Proc
  MRBC_TT_ARRAY     = 11,       //!< Array
  MRBC_TT_STRING    = 12,       //!< String
  MRBC_TT_RANGE     = 13,       //!< Range
  MRBC_TT_HASH      = 14,       //!< Hash
  MRBC_TT_EXCEPTION = 15,       //!< Exception
} mrbc_vtype;
#define	MRBC_TT_INC_DEC_THRESHOLD MRBC_TT_MODULE
#define	MRBC_TT_MAXVAL MRBC_TT_EXCEPTION


//================================================================
/*! error code for internal use. (BETA TEST)
*/
typedef enum {
  E_NOMEMORY_ERROR = 1,
  E_RUNTIME_ERROR,
  E_TYPE_ERROR,
  E_ARGUMENT_ERROR,
  E_INDEX_ERROR,
  E_RANGE_ERROR,
  E_NAME_ERROR,
  E_NOMETHOD_ERROR,
  E_SCRIPT_ERROR,
  E_SYNTAX_ERROR,
  E_LOCALJUMP_ERROR,
  E_REGEXP_ERROR,
  E_NOTIMP_ERROR,
  E_FLOATDOMAIN_ERROR,
  E_KEY_ERROR,

  // Internal Error
  E_BYTECODE_ERROR,
} mrbc_error_code;


//================================================================
/* Define the object structure having reference counter.
*/
#if defined(MRBC_DEBUG)
#define MRBC_OBJECT_HEADER  uint8_t obj_mark_[2]; uint16_t ref_count
#else
#define MRBC_OBJECT_HEADER  uint16_t ref_count
#endif

//================================================================
/*!@brief
  Base class for some objects.
*/
struct RBasic {
  MRBC_OBJECT_HEADER;
};


//================================================================
/*!@brief
  Value object.
*/
struct RObject {
  mrbc_vtype tt : 8;
  union {
    mrbc_int_t i;		// MRBC_TT_INTEGER
#if MRBC_USE_FLOAT
    mrbc_float_t d;		// MRBC_TT_FLOAT
#endif
    mrbc_sym sym_id;		// MRBC_TT_SYMBOL
    struct RBasic *obj;		// use inc/dec ref only.
    struct RClass *cls;		// MRBC_TT_CLASS, MRBC_TT_MODULE
    struct RInstance *instance;	// MRBC_TT_OBJECT
    struct RProc *proc;		// MRBC_TT_PROC
    struct RArray *array;	// MRBC_TT_ARRAY
    struct RString *string;	// MRBC_TT_STRING
    struct RRange *range;	// MRBC_TT_RANGE
    struct RHash *hash;		// MRBC_TT_HASH
    struct RException *exception; // MRBC_TT_EXCEPTION
    void *handle;		// internal use only.
  };
};
typedef struct RObject mrb_object;	// not recommended.
typedef struct RObject mrb_value;	// not recommended.
typedef struct RObject mrbc_object;
typedef struct RObject mrbc_value;


/***** Macros ***************************************************************/

// getters
/*!
  @def mrbc_type(o)
  get the type (#mrbc_vtype) from mrbc_value.

  @def mrbc_integer(o)
  get int value from mrbc_value.

  @def mrbc_float(o)
  get float(double) value from mrbc_value.

  @def mrbc_symbol(o)
  get symbol value (#mrbc_sym) from mrbc_value.
*/
#define mrbc_type(o)		((o).tt)
#define mrbc_integer(o)		((o).i)
#define mrbc_float(o)		((o).d)
#define mrbc_symbol(o)		((o).sym_id)

// setters
#define mrbc_set_integer(p,n)	(p)->tt = MRBC_TT_INTEGER; (p)->i = (n)
#define mrbc_set_float(p,n)	(p)->tt = MRBC_TT_FLOAT; (p)->d = (n)
#define mrbc_set_nil(p)		(p)->tt = MRBC_TT_NIL
#define mrbc_set_true(p)	(p)->tt = MRBC_TT_TRUE
#define mrbc_set_false(p)	(p)->tt = MRBC_TT_FALSE
#define mrbc_set_bool(p,n)	(p)->tt = (n)? MRBC_TT_TRUE: MRBC_TT_FALSE
#define mrbc_set_symbol(p,n)	(p)->tt = MRBC_TT_SYMBOL; (p)->sym_id = (n)

// make immediate values.
#define mrbc_integer_value(n)	((mrbc_value){.tt = MRBC_TT_INTEGER, .i=(n)})
#define mrbc_float_value(vm,n)	((mrbc_value){.tt = MRBC_TT_FLOAT, .d=(n)})
#define mrbc_nil_value()	((mrbc_value){.tt = MRBC_TT_NIL})
#define mrbc_true_value()	((mrbc_value){.tt = MRBC_TT_TRUE})
#define mrbc_false_value()	((mrbc_value){.tt = MRBC_TT_FALSE})
#define mrbc_bool_value(n)	((mrbc_value){.tt = (n)?MRBC_TT_TRUE:MRBC_TT_FALSE})
#define mrbc_symbol_value(n)	((mrbc_value){.tt = MRBC_TT_SYMBOL, .sym_id=(n)})

// (for mruby compatible)
#define mrb_type(o)		mrbc_type(o)
#define mrb_integer(o)		mrbc_integer(o)
#define mrb_float(o)		mrbc_float(o)
#define mrb_symbol(o)		mrbc_symbol(o)
#define mrb_integer_value(n)	mrbc_integer_value(n)
#define mrb_float_value(vm,n)	mrbc_float_value(vm,n)
#define mrb_nil_value()		mrbc_nil_value()
#define mrb_true_value()	mrbc_true_value()
#define mrb_false_value()	mrbc_false_value()
#define mrb_bool_value(n)	mrbc_bool_value(n)
#define mrb_symbol_value(n)	mrbc_symbol_value(n)

// (for mruby/c 2 compatibility)
#define mrbc_fixnum(o)		mrbc_integer(o)
#define mrbc_set_fixnum(p,n)	mrbc_set_integer(p,n)
#define mrbc_fixnum_value(n)	mrbc_integer_value(n)
#define mrb_fixnum(o)		mrbc_integer(o)
#define mrb_fixnum_value(n)	mrbc_integer_value(n)



// for C call
/**
  @def SET_RETURN(n)
  set a return value when writing a method by C.

  @def SET_NIL_RETURN()
  set a return value to nil when writing a method by C.

  @def SET_FALSE_RETURN()
  set a return value to false when writing a method by C.

  @def SET_TRUE_RETURN()
  set a return value to true when writing a method by C.

  @def SET_BOOL_RETURN(n)
  set a return value to true or false when writing a method by C.

  @def SET_INT_RETURN(n)
  set an integer return value when writing a method by C.

  @def SET_FLOAT_RETURN(n)
  set a float return value when writing a method by C.
*/
#define SET_RETURN(n) do {	\
    mrbc_value nnn = (n);	\
    mrbc_decref(v);		\
    v[0] = nnn;			\
  } while(0)
#define SET_NIL_RETURN() do {	\
    mrbc_decref(v);		\
    v[0].tt = MRBC_TT_NIL;	\
  } while(0)
#define SET_FALSE_RETURN() do { \
    mrbc_decref(v);		\
    v[0].tt = MRBC_TT_FALSE;	\
  } while(0)
#define SET_TRUE_RETURN() do {	\
    mrbc_decref(v);		\
    v[0].tt = MRBC_TT_TRUE;	\
  } while(0)
#define SET_BOOL_RETURN(n) do {			 \
    int tt = (n) ? MRBC_TT_TRUE : MRBC_TT_FALSE; \
    mrbc_decref(v);				 \
    v[0].tt = tt;				 \
  } while(0)
#define SET_INT_RETURN(n) do {	\
    mrbc_int_t nnn = (n);	\
    mrbc_decref(v);		\
    v[0].tt = MRBC_TT_INTEGER;	\
    v[0].i = nnn;		\
  } while(0)
#define SET_FLOAT_RETURN(n) do {\
    mrbc_float_t nnn = (n);	\
    mrbc_decref(v);		\
    v[0].tt = MRBC_TT_FLOAT;	\
    v[0].d = nnn;		\
} while(0)

#define GET_TT_ARG(n)		(v[(n)].tt)
#define GET_INT_ARG(n)		(v[(n)].i)
#define GET_ARY_ARG(n)		(v[(n)])
#define GET_ARG(n)		(v[(n)])
#define GET_FLOAT_ARG(n)	(v[(n)].d)
#define GET_STRING_ARG(n)	(v[(n)].string->data)


#if defined(MRBC_DEBUG)
#define MRBC_INIT_OBJECT_HEADER(p, t)  (p)->ref_count = 1; (p)->obj_mark_[0] = (t)[0]; (p)->obj_mark_[1] = (t)[1]
#else
#define MRBC_INIT_OBJECT_HEADER(p, t)  (p)->ref_count = 1
#endif


// for Numeric values.
/*!
  @def MRBC_ISNUMERIC(val)
  Check the val is numeric.

  @def MRBC_TO_INT(val)
  Convert mrbc_value to C-lang int.

  @def MRBC_TO_FLOAT(val)
  Convert mrbc_value to C-lang double.
*/
#define MRBC_ISNUMERIC(val) \
  ((val).tt == MRBC_TT_INTEGER || (val).tt == MRBC_TT_FLOAT)
#define MRBC_TO_INT(val) \
  (val).tt == MRBC_TT_INTEGER ? (val).i : \
  (val).tt == MRBC_TT_FLOAT ? (mrbc_int_t)(val).d : 0
#define MRBC_TO_FLOAT(val) \
  (val).tt == MRBC_TT_FLOAT ? (val).d : \
  (val).tt == MRBC_TT_INTEGER ? (mrbc_float_t)(val).i : 0.0


// Method argument getters.
/*!
  @def MRBC_ARG
  (beta) Get a N'th argument pointer.

  <b>Examples:</b>
  @code
  mrbc_value *arg1 = MRBC_ARG(1);	// Gets a 1st argument pointer.
  @endcode


  @def MRBC_ARG_I
  (beta) Get a N'th argument as a C integer.

  <b>Examples:</b>
  @code
  int arg1 = MRBC_ARG_I(1);		// Get a 1st argument as an integer.
  int arg1 = MRBC_ARG_I(1, 111);	// with default value 111.
  @endcode


  @def MRBC_ARG_F
  (beta) Get a N'th argument as a C float (double).

  <b>Examples:</b>
  @code
  double arg1 = MRBC_ARG_F(1);		// Get a 1st argument as an double.
  double arg1 = MRBC_ARG_F(1, 2.7);	// with default value 2.7.
  @endcode


  @def MRBC_ARG_S
  (beta) Get a N'th argument as a C string.

  <b>Examples:</b>
  @code
  const char *arg1 = MRBC_ARG_S(1);	// Get a 1st argument as an double.
  const char *arg1 = MRBC_ARG_S(1,"DEFAULT"); // with default value "DEFAULT".
  @endcode


  @def MRBC_ARG_B
  (beta) Get a N'th True/False argument as a C integer.

  <b>Examples:</b>
  @code
  int arg1 = MRBC_ARG_B(1);	// Get a 1st True/False argument as an integer.
  int arg1 = MRBC_ARG_B(1, 0);	// with default value False.
  @endcode
*/
#define MRBC_ARG(n) mrbc_arg(vm, v, argc, (n))
#define MRBC_ARG_I(...) MRBC_arg_choice(__VA_ARGS__, mrbc_arg_i2, mrbc_arg_i) (vm,v,argc,__VA_ARGS__)
#define MRBC_ARG_F(...) MRBC_arg_choice(__VA_ARGS__, mrbc_arg_f2, mrbc_arg_f) (vm,v,argc,__VA_ARGS__)
#define MRBC_ARG_S(...) MRBC_arg_choice(__VA_ARGS__, mrbc_arg_s2, mrbc_arg_s) (vm,v,argc,__VA_ARGS__)
#define MRBC_ARG_B(...) MRBC_arg_choice(__VA_ARGS__, mrbc_arg_b2, mrbc_arg_b) (vm,v,argc,__VA_ARGS__)
//@cond
#define MRBC_arg_choice(a1,a2,a3,...) a3
//@endcond


// mrbc_value converters.
/*!
  @def MRBC_TO_I(mrbc_value *val)
  (beta) Convert mrbc_value to Integer.

  @def MRBC_TO_F(mrbc_value *val)
  (beta) Convert mrbc_value to Float.

  @def MRBC_TO_S(mrbc_value *val)
  (beta) Convert mrbc_value to String.
*/
#define MRBC_TO_I(val) mrbc_to_i(vm, v, argc, val)
#define MRBC_TO_F(val) mrbc_to_f(vm, v, argc, val)
#define MRBC_TO_S(val) mrbc_to_s(vm, v, argc, val)


// for keyword arguments
/*!
  @def MRBC_KW_ARG(keyword1,...)
  Get keyword arguments and define mrbc_value with same name.
  Up to 30 arguments can be specified.

  @def MRBC_KW_DICT(dict_var)
  Get remaining keyword arguments as hash.

  @def MRBC_KW_ISVALID(mrbc_value)
  Check if argument is valid.

  @def MRBC_KW_MANDATORY(keyword1,...)
  Check if mandatory keyword arguments are given.
  If not, return False(=0) and set ArgumentError.

  @def MRBC_KW_END()
  Check for excess keyword arguments.
  If excess keyword argument are given, return False(=0) and set ArgumentError.

  @def MRBC_KW_DELETE(mrbc_value1,...)
  Delete retrieved keyword arguments.
*/
#define MRBC_KW_ARG(...) \
  MRBC_each(__VA_ARGS__)( MRBC_KW_ARG_decl1, __VA_ARGS__ ) \
  if( v[argc+1].tt == MRBC_TT_HASH ) { \
    MRBC_each(__VA_ARGS__)( MRBC_KW_ARG_decl2, __VA_ARGS__ ) \
  }
#define MRBC_KW_ARG_decl1(kw) mrbc_value kw = {.tt = MRBC_TT_EMPTY};
#define MRBC_KW_ARG_decl2(kw) kw = mrbc_hash_remove_by_id(&v[argc+1], mrbc_str_to_symid(#kw));

#define MRBC_KW_DICT(dict) \
  mrbc_value dict; \
  if( v[argc+1].tt == MRBC_TT_HASH ) { dict = v[argc+1]; v[argc+1].tt = MRBC_TT_EMPTY; } \
  else { dict = mrbc_hash_new(vm, 0); }

#define MRBC_KW_ISVALID(kw) (kw.tt != MRBC_TT_EMPTY)

#define MRBC_KW_MANDATORY(...) \
  (MRBC_each(__VA_ARGS__)( MRBC_KW_MANDATORY_decl1, __VA_ARGS__ ) 1)
#define MRBC_KW_MANDATORY_decl1(kw) (MRBC_KW_ISVALID(kw)? 1 : \
  (mrbc_raisef(vm, MRBC_CLASS(ArgumentError), "missing keyword: %s", #kw), 0))&&

#define MRBC_KW_END() \
  (((v[argc+1].tt == MRBC_TT_HASH) && mrbc_hash_size(&v[argc+1])) ? \
   (mrbc_raise(vm, MRBC_CLASS(ArgumentError), "unknown keyword"), 0) : 1)

#define MRBC_KW_DELETE(...) \
  MRBC_each(__VA_ARGS__)( MRBC_KW_DELETE_decl, __VA_ARGS__ )
#define MRBC_KW_DELETE_decl(kw) mrbc_decref(&kw);

//@cond
#define MRBC_each(...) MRBC_each_sel(__VA_ARGS__, \
  MRBC_each30,MRBC_each29,MRBC_each28,MRBC_each27,MRBC_each26, \
  MRBC_each25,MRBC_each24,MRBC_each23,MRBC_each22,MRBC_each21, \
  MRBC_each20,MRBC_each19,MRBC_each18,MRBC_each17,MRBC_each16, \
  MRBC_each15,MRBC_each14,MRBC_each13,MRBC_each12,MRBC_each11, \
  MRBC_each10,MRBC_each9, MRBC_each8, MRBC_each7, MRBC_each6,  \
  MRBC_each5, MRBC_each4, MRBC_each3, MRBC_each2, MRBC_each1)
#define MRBC_each_sel(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30, a31, ...) a31
#define MRBC_each1(func,arg) func(arg)
#define MRBC_each2(func,arg, ...) func(arg) MRBC_each1(func,__VA_ARGS__)
#define MRBC_each3(func,arg, ...) func(arg) MRBC_each2(func,__VA_ARGS__)
#define MRBC_each4(func,arg, ...) func(arg) MRBC_each3(func,__VA_ARGS__)
#define MRBC_each5(func,arg, ...) func(arg) MRBC_each4(func,__VA_ARGS__)
#define MRBC_each6(func,arg, ...) func(arg) MRBC_each5(func,__VA_ARGS__)
#define MRBC_each7(func,arg, ...) func(arg) MRBC_each6(func,__VA_ARGS__)
#define MRBC_each8(func,arg, ...) func(arg) MRBC_each7(func,__VA_ARGS__)
#define MRBC_each9(func,arg, ...) func(arg) MRBC_each8(func,__VA_ARGS__)
#define MRBC_each10(func,arg, ...) func(arg) MRBC_each9(func,__VA_ARGS__)
#define MRBC_each11(func,arg, ...) func(arg) MRBC_each10(func,__VA_ARGS__)
#define MRBC_each12(func,arg, ...) func(arg) MRBC_each11(func,__VA_ARGS__)
#define MRBC_each13(func,arg, ...) func(arg) MRBC_each12(func,__VA_ARGS__)
#define MRBC_each14(func,arg, ...) func(arg) MRBC_each13(func,__VA_ARGS__)
#define MRBC_each15(func,arg, ...) func(arg) MRBC_each14(func,__VA_ARGS__)
#define MRBC_each16(func,arg, ...) func(arg) MRBC_each15(func,__VA_ARGS__)
#define MRBC_each17(func,arg, ...) func(arg) MRBC_each16(func,__VA_ARGS__)
#define MRBC_each18(func,arg, ...) func(arg) MRBC_each17(func,__VA_ARGS__)
#define MRBC_each19(func,arg, ...) func(arg) MRBC_each18(func,__VA_ARGS__)
#define MRBC_each20(func,arg, ...) func(arg) MRBC_each19(func,__VA_ARGS__)
#define MRBC_each21(func,arg, ...) func(arg) MRBC_each20(func,__VA_ARGS__)
#define MRBC_each22(func,arg, ...) func(arg) MRBC_each21(func,__VA_ARGS__)
#define MRBC_each23(func,arg, ...) func(arg) MRBC_each22(func,__VA_ARGS__)
#define MRBC_each24(func,arg, ...) func(arg) MRBC_each23(func,__VA_ARGS__)
#define MRBC_each25(func,arg, ...) func(arg) MRBC_each24(func,__VA_ARGS__)
#define MRBC_each26(func,arg, ...) func(arg) MRBC_each25(func,__VA_ARGS__)
#define MRBC_each27(func,arg, ...) func(arg) MRBC_each26(func,__VA_ARGS__)
#define MRBC_each28(func,arg, ...) func(arg) MRBC_each27(func,__VA_ARGS__)
#define MRBC_each29(func,arg, ...) func(arg) MRBC_each28(func,__VA_ARGS__)
#define MRBC_each30(func,arg, ...) func(arg) MRBC_each29(func,__VA_ARGS__)
//@endcond


/*!
  @def MRBC_PTR_TO_UINT32(p)
  convert pointer to uint32_t
*/
#if defined(UINTPTR_MAX)
#define MRBC_PTR_TO_UINT32(p) ((uint32_t)(uintptr_t)(p))
#else
#define MRBC_PTR_TO_UINT32(p) ((uint32_t)(p))
#endif


/***** Global variables *****************************************************/
extern void (* const mrbc_delfunc[])(mrbc_value *);


/***** Function prototypes **************************************************/
//@cond
int mrbc_compare(const mrbc_value *v1, const mrbc_value *v2);
void mrbc_clear_vm_id(mrbc_value *v);
mrbc_int_t mrbc_atoi(const char *s, int base);
int mrbc_strcpy(char *dest, int destsize, const char *src);
mrbc_value *mrbc_arg(struct VM *vm, mrbc_value v[], int argc, int n);
mrbc_int_t mrbc_arg_i(struct VM *vm, mrbc_value v[], int argc, int n);
mrbc_int_t mrbc_arg_i2(struct VM *vm, mrbc_value v[], int argc, int n, mrbc_int_t default_value);
mrbc_float_t mrbc_arg_f(struct VM *vm, mrbc_value v[], int argc, int n);
mrbc_float_t mrbc_arg_f2(struct VM *vm, mrbc_value v[], int argc, int n, mrbc_float_t default_value);
const char *mrbc_arg_s(struct VM *vm, mrbc_value v[], int argc, int n);
const char *mrbc_arg_s2(struct VM *vm, mrbc_value v[], int argc, int n, const char *default_value);
int mrbc_arg_b(struct VM *vm, mrbc_value v[], int argc, int n);
int mrbc_arg_b2(struct VM *vm, mrbc_value v[], int argc, int n, int default_value);
mrbc_int_t mrbc_to_i(struct VM *vm, mrbc_value v[], int argc, mrbc_value *val);
mrbc_float_t mrbc_to_f(struct VM *vm, mrbc_value v[], int argc, mrbc_value *val);
char *mrbc_to_s(struct VM *vm, mrbc_value v[], int argc, mrbc_value *val);
//@endcond


/***** Inline functions *****************************************************/

//================================================================
/*! Increment reference counter

  @param   v     Pointer to mrbc_value
*/
static inline void mrbc_incref(mrbc_value *v)
{
  if( v->tt <= MRBC_TT_INC_DEC_THRESHOLD ) return;

  assert( v->obj->ref_count != 0 );
  assert( v->obj->ref_count != 0xff );	// check max value.
  v->obj->ref_count++;
}


//================================================================
/*! Decrement reference counter

  @param   v     Pointer to target mrbc_value
*/
static inline void mrbc_decref(mrbc_value *v)
{
  if( v->tt <= MRBC_TT_INC_DEC_THRESHOLD ) return;

  assert( v->obj->ref_count != 0 );
  assert( v->obj->ref_count != 0xffff );	// check broken data.

  if( --v->obj->ref_count != 0 ) return;

  (*mrbc_delfunc[v->tt])(v);
}


//================================================================
/*! Decrement reference counter with set TT_EMPTY.

  @param   v     Pointer to target mrbc_value
*/
static inline void mrbc_decref_empty(mrbc_value *v)
{
  mrbc_decref(v);
  v->tt = MRBC_TT_EMPTY;
}


//================================================================
/*! delete value but same as mrbc_decref() function.

  @param   v     Pointer to target mrbc_value
*/
static inline void mrbc_delete(mrbc_value *v)
{
  mrbc_decref_empty(v);
}


//================================================================
/*! Get 16bit int value from memory.

  @param  s	Pointer to memory.
  @return	16bit unsigned int value.
*/
static inline uint16_t bin_to_uint16( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Little endian, no alignment.
  uint16_t x = *((uint16_t *)s);
  x = (x << 8) | (x >> 8);

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Big endian, no alignment.
  uint16_t x = *((uint16_t *)s);

#elif defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // 32bit alignment required.
  uint8_t *p = (uint8_t *)s;
  uint16_t x = *p++;
  x <<= 8;
  x |= *p;

#else
  #error "Specify MRBC_BIG_ENDIAN or MRBC_LITTLE_ENDIAN"
#endif

  return x;
}


//================================================================
/*! Get 32bit int value from memory.

  @param  s	Pointer to memory.
  @return	32bit unsigned int value.
*/
static inline uint32_t bin_to_uint32( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Little endian, no alignment.
  uint32_t x = *((uint32_t *)s);
  x = (x << 24) | ((x & 0xff00) << 8) | ((x >> 8) & 0xff00) | (x >> 24);

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Big endian, no alignment.
  uint32_t x = *((uint32_t *)s);

#elif defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // 32bit alignment required.
  uint8_t *p = (uint8_t *)s;
  uint32_t x = *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p;

#endif

  return x;
}


#if defined(MRBC_INT64)
//================================================================
/*! Get 64bit int value from memory.

  @param  s	Pointer to memory.
  @return	64bit int value.
*/
static inline int64_t bin_to_int64( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Little endian, no alignment.
  uint64_t x = *((uint64_t *)s);
  uint64_t y = (x << 56);
  y |= ((uint64_t)(uint8_t)(x >>  8)) << 48;
  y |= ((uint64_t)(uint8_t)(x >> 16)) << 40;
  y |= ((uint64_t)(uint8_t)(x >> 24)) << 32;
  y |= ((uint64_t)(uint8_t)(x >> 32)) << 24;
  y |= ((uint64_t)(uint8_t)(x >> 40)) << 16;
  y |= ((uint64_t)(uint8_t)(x >> 48)) << 8;
  y |= (x >> 56);
  return y;

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Big endian, no alignment.
  return *((uint64_t *)s);

#elif defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // 64bit alignment required.
  uint8_t *p = (uint8_t *)s;
  uint64_t x = *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p;
  return x;

#endif

}
#endif


//================================================================
/*! Get double (64bit) value from memory.

  @param  s	Pointer to memory.
  @return	double value.
*/
static inline double bin_to_double64( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Little endian, no alignment.
  return *((double *)s);

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Big endian, no alignment.
  double x;
  uint8_t *p1 = (uint8_t*)s;
  uint8_t *p2 = (uint8_t*)&x + 7;
  int i;
  for( i = 7; i >= 0; i-- ) {
    *p2-- = *p1++;
  }
  return x;

#elif defined(MRBC_LITTLE_ENDIAN) && defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Little endian, 64bit alignment required.
  double x;
  uint8_t *p1 = (uint8_t*)s;
  uint8_t *p2 = (uint8_t*)&x;
  int i;
  for( i = 7; i >= 0; i-- ) {
    *p2++ = *p1++;
  }
  return x;

#elif defined(MRBC_BIG_ENDIAN) && defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Big endian, 64bit alignment required.
  double x;
  uint8_t *p1 = (uint8_t*)s;
  uint8_t *p2 = (uint8_t*)&x + 7;
  int i;
  for( i = 7; i >= 0; i-- ) {
    *p2-- = *p1++;
  }
  return x;

#endif
}


//================================================================
/*! Set 32bit value to memory.

  @param  v	Source value.
  @param  d	Pointer to memory.
*/
static inline void uint32_to_bin( uint32_t v, void *d )
{
  uint8_t *p = (uint8_t *)d + 3;
  *p-- = 0xff & v;
  v >>= 8;
  *p-- = 0xff & v;
  v >>= 8;
  *p-- = 0xff & v;
  v >>= 8;
  *p = 0xff & v;
}


//================================================================
/*! Set 16bit value to memory.

  @param  v	Source value.
  @param  d	Pointer to memory.
*/
static inline void uint16_to_bin( uint16_t v, void *d )
{
  uint8_t *p = (uint8_t *)d;
  *p++ = (v >> 8);
  *p = 0xff & v;
}


#ifdef __cplusplus
}
#endif
#endif

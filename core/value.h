/*! @file
  @brief
  mruby/c value definitions

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#ifndef MRBC_SRC_VALUE_H_
#define MRBC_SRC_VALUE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// pre define of some struct
struct VM;
struct IREP;
struct RObject;
struct RClass;
struct RInstance;
struct RProc;

// mrbc types
typedef int32_t mrbc_int;
typedef double mrbc_float;
typedef int16_t mrbc_sym;
typedef void (*mrbc_func_t)(struct VM *vm, struct RObject *v, int argc);

/* aspec access ? */
#define MRB_ASPEC_REQ(a)          (((a) >> 18) & 0x1f)
#define MRB_ASPEC_OPT(a)          (((a) >> 13) & 0x1f)
#define MRB_ASPEC_REST(a)         (((a) >> 12) & 0x1)
#define MRB_ASPEC_POST(a)         (((a) >> 7) & 0x1f)

#define MRBC_OBJECT_HEADER \
  uint16_t ref_count; \
  mrbc_vtype tt : 8  // TODO: for debug use only.



//================================================================
/*!@brief
  define the value type.
*/
typedef enum {
  /* internal use */
  MRBC_TT_HANDLE = -1,
  /* primitive */
  MRBC_TT_EMPTY = 0,
  MRBC_TT_NIL,
  MRBC_TT_FALSE,		// (note) true/false threshold. see op_jmpif

  MRBC_TT_TRUE,
  MRBC_TT_FIXNUM,
  MRBC_TT_FLOAT,
  MRBC_TT_SYMBOL,
  MRBC_TT_CLASS,

  /* non-primitive */
  MRBC_TT_OBJECT = 20,
  MRBC_TT_PROC,
  MRBC_TT_ARRAY,
  MRBC_TT_STRING,
  MRBC_TT_RANGE,
  MRBC_TT_HASH,

} mrbc_vtype;


//================================================================
/*!@brief
  define the error code. (BETA TEST)
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
} mrbc_error_code;



//================================================================
/*!@brief
  mruby/c value object.
*/
struct RObject {
  mrbc_vtype tt : 8;
  union {
    mrbc_int i;			// MRBC_TT_FIXNUM, SYMBOL
#if MRBC_USE_FLOAT
    mrbc_float d;		// MRBC_TT_FLOAT
#endif
    struct RClass *cls;		// MRBC_TT_CLASS
    struct RObject *handle;	// handle to objects
    struct RInstance *instance;	// MRBC_TT_OBJECT
    struct RProc *proc;		// MRBC_TT_PROC
    struct RArray *array;	// MRBC_TT_ARRAY
    struct RString *string;	// MRBC_TT_STRING
    const char *str;		// C-string (only loader use.)
    struct RRange *range;	// MRBC_TT_RANGE
    struct RHash *hash;		// MRBC_TT_HASH
  };
};
typedef struct RObject mrb_object;	// not recommended.
typedef struct RObject mrb_value;	// not recommended.
typedef struct RObject mrbc_object;
typedef struct RObject mrbc_value;




// for C call
#define SET_RETURN(n)		do { mrbc_value nnn = (n); \
    mrbc_dec_ref_counter(v); v[0] = nnn; } while(0)
#define SET_NIL_RETURN()	do { \
    mrbc_dec_ref_counter(v); v[0].tt = MRBC_TT_NIL; } while(0)
#define SET_FALSE_RETURN()	do { \
    mrbc_dec_ref_counter(v); v[0].tt = MRBC_TT_FALSE; } while(0)
#define SET_TRUE_RETURN()	do { \
    mrbc_dec_ref_counter(v); v[0].tt = MRBC_TT_TRUE; } while(0)
#define SET_BOOL_RETURN(n)	do { \
    mrbc_dec_ref_counter(v); v[0].tt = (n)?MRBC_TT_TRUE:MRBC_TT_FALSE; } while(0)
#define SET_INT_RETURN(n)	do { mrbc_int nnn = (n);		\
    mrbc_dec_ref_counter(v); v[0].tt = MRBC_TT_FIXNUM; v[0].i = nnn; } while(0)
#define SET_FLOAT_RETURN(n)	do { mrbc_float nnn = (n); \
    mrbc_dec_ref_counter(v); v[0].tt = MRBC_TT_FLOAT; v[0].d = nnn; } while(0)

#define GET_TT_ARG(n)		(v[(n)].tt)
#define GET_INT_ARG(n)		(v[(n)].i)
#define GET_ARY_ARG(n)		(v[(n)])
#define GET_ARG(n)		(v[(n)])
#define GET_FLOAT_ARG(n)	(v[(n)].d)
#define GET_STRING_ARG(n)	(v[(n)].string->data)

#define mrbc_fixnum_value(n)	((mrbc_value){.tt = MRBC_TT_FIXNUM, .i=(n)})
#define mrbc_float_value(n)	((mrbc_value){.tt = MRBC_TT_FLOAT, .d=(n)})
#define mrbc_nil_value()	((mrbc_value){.tt = MRBC_TT_NIL})
#define mrbc_true_value()	((mrbc_value){.tt = MRBC_TT_TRUE})
#define mrbc_false_value()	((mrbc_value){.tt = MRBC_TT_FALSE})
#define mrbc_bool_value(n)	((mrbc_value){.tt = (n)?MRBC_TT_TRUE:MRBC_TT_FALSE})

int mrbc_compare(const mrbc_value *v1, const mrbc_value *v2);
void mrbc_dup(mrbc_value *v);
void mrbc_release(mrbc_value *v);
void mrbc_dec_ref_counter(mrbc_value *v);
void mrbc_clear_vm_id(mrbc_value *v);
mrbc_int mrbc_atoi(const char *s, int base);


// (mruby compatible functions.)

//================================================================
/*!@brief
  Returns a fixnum in mruby/c.

  @param  n	int value
  @return	mrbc_value of type fixnum.
*/
static inline mrbc_value mrb_fixnum_value( mrbc_int n )
{
  mrbc_value value = {.tt = MRBC_TT_FIXNUM};
  value.i = n;
  return value;
}


#if MRBC_USE_FLOAT
//================================================================
/*!@brief
  Returns a float in mruby/c.

  @param  n	dluble value
  @return	mrbc_value of type float.
*/
static inline mrbc_value mrb_float_value( mrbc_float n )
{
  mrbc_value value = {.tt = MRBC_TT_FLOAT};
  value.d = n;
  return value;
}
#endif


//================================================================
/*!@brief
  Returns a nil in mruby/c.

  @return	mrbc_value of type nil.
*/
static inline mrbc_value mrb_nil_value(void)
{
  mrbc_value value = {.tt = MRBC_TT_NIL};
  return value;
}


//================================================================
/*!@brief
  Returns a true in mruby/c.

  @return	mrbc_value of type true.
*/
static inline mrbc_value mrb_true_value(void)
{
  mrbc_value value = {.tt = MRBC_TT_TRUE};
  return value;
}


//================================================================
/*!@brief
  Returns a false in mruby/c.

  @return	mrbc_value of type false.
*/
static inline mrbc_value mrb_false_value(void)
{
  mrbc_value value = {.tt = MRBC_TT_FALSE};
  return value;
}


//================================================================
/*!@brief
  Returns a true or false in mruby/c.

  @return	mrbc_value of type false.
*/
static inline mrbc_value mrb_bool_value( int n )
{
  mrbc_value value = {.tt = n ? MRBC_TT_TRUE : MRBC_TT_FALSE};
  return value;
}


#ifdef __cplusplus
}
#endif
#endif

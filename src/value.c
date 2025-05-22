/*! @file
  @brief
  mruby/c value definitions

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
//================================================================
/*! function table for object delete.

  @note must be same order as mrbc_vtype.
  @see mrbc_vtype in value.h
*/
void (* const mrbc_delfunc[])(mrbc_value *) = {
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  mrbc_instance_delete,         // MRBC_TT_OBJECT    = 9,
  mrbc_proc_delete,             // MRBC_TT_PROC      = 10,
  mrbc_array_delete,            // MRBC_TT_ARRAY     = 11,
#if MRBC_USE_STRING
  mrbc_string_delete,           // MRBC_TT_STRING    = 12,
#else
  NULL,
#endif
  mrbc_range_delete,            // MRBC_TT_RANGE     = 13,
  mrbc_hash_delete,             // MRBC_TT_HASH      = 14,
  mrbc_exception_delete,        // MRBC_TT_EXCEPTION = 15,
};


/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! compare two mrbc_values

  @param  v1	Pointer to mrbc_value
  @param  v2	Pointer to another mrbc_value
  @retval 0	v1 == v2
  @retval plus	v1 >  v2
  @retval minus	v1 <  v2
*/
int mrbc_compare(const mrbc_value *v1, const mrbc_value *v2)
{
#if MRBC_USE_FLOAT
  mrbc_float_t d1, d2;
#endif

  // if TT_XXX is different
  if( mrbc_type(*v1) != mrbc_type(*v2) ) {
#if MRBC_USE_FLOAT
    // but Numeric?
    if( mrbc_type(*v1) == MRBC_TT_INTEGER && mrbc_type(*v2) == MRBC_TT_FLOAT ) {
      d1 = v1->i;
      d2 = v2->d;
      goto CMP_FLOAT;
    }
    if( mrbc_type(*v1) == MRBC_TT_FLOAT && mrbc_type(*v2) == MRBC_TT_INTEGER ) {
      d1 = v1->d;
      d2 = v2->i;
      goto CMP_FLOAT;
    }
#endif

    // leak Empty?
    if((mrbc_type(*v1) == MRBC_TT_EMPTY && mrbc_type(*v2) == MRBC_TT_NIL) ||
       (mrbc_type(*v1) == MRBC_TT_NIL   && mrbc_type(*v2) == MRBC_TT_EMPTY)) return 0;

    // other case
    return mrbc_type(*v1) - mrbc_type(*v2);
  }

  // check value
  switch( mrbc_type(*v1) ) {
  case MRBC_TT_NIL:
  case MRBC_TT_FALSE:
  case MRBC_TT_TRUE:
    return 0;

  case MRBC_TT_INTEGER:
    return mrbc_integer(*v1) - mrbc_integer(*v2);

  case MRBC_TT_SYMBOL: {
    const char *str1 = mrbc_symid_to_str(mrbc_symbol(*v1));
    const char *str2 = mrbc_symid_to_str(mrbc_symbol(*v2));
    int diff = strlen(str1) - strlen(str2);
    int len = diff < 0 ? strlen(str1) : strlen(str2);
    int res = memcmp(str1, str2, len);
    return (res != 0) ? res : diff;
  }

#if MRBC_USE_FLOAT
  case MRBC_TT_FLOAT:
    d1 = mrbc_float(*v1);
    d2 = mrbc_float(*v2);
    goto CMP_FLOAT;
#endif

  case MRBC_TT_CLASS:
  case MRBC_TT_MODULE:
  case MRBC_TT_OBJECT:
  case MRBC_TT_PROC:
    return (v1->cls > v2->cls) * 2 - (v1->cls != v2->cls);

  case MRBC_TT_ARRAY:
    return mrbc_array_compare( v1, v2 );

#if MRBC_USE_STRING
  case MRBC_TT_STRING:
    return mrbc_string_compare( v1, v2 );
#endif

  case MRBC_TT_RANGE:
    return mrbc_range_compare( v1, v2 );

  case MRBC_TT_HASH:
    return mrbc_hash_compare( v1, v2 );

  default:
    return 1;
  }

#if MRBC_USE_FLOAT
 CMP_FLOAT:
  return -1 + (d1 == d2) + (d1 > d2)*2;	// caution: NaN == NaN is false
#endif
}


#if defined(MRBC_ALLOC_VMID)
//================================================================
/*! clear vm id

  @param   v     Pointer to target mrbc_value
*/
void mrbc_clear_vm_id(mrbc_value *v)
{
  switch( mrbc_type(*v) ) {
  case MRBC_TT_OBJECT:	mrbc_instance_clear_vm_id(v);	break;
  case MRBC_TT_PROC:	mrbc_proc_clear_vm_id(v);	break;
  case MRBC_TT_ARRAY:	mrbc_array_clear_vm_id(v);	break;
#if MRBC_USE_STRING
  case MRBC_TT_STRING:	mrbc_string_clear_vm_id(v);	break;
#endif
  case MRBC_TT_RANGE:	mrbc_range_clear_vm_id(v);	break;
  case MRBC_TT_HASH:	mrbc_hash_clear_vm_id(v);	break;

  default:
    // Nothing
    break;
  }
}
#endif


//================================================================
/*! convert ASCII string to integer mruby/c version

  @param  s	source string.
  @param  base	n base.
  @return	result.
*/
mrbc_int_t mrbc_atoi( const char *s, int base )
{
  int ret = 0;
  int sign = 0;

 REDO:
  switch( *s ) {
  case '-':
    sign = 1;
    // fall through.
  case '+':
    s++;
    break;

  case ' ':
    s++;
    goto REDO;
  }

  int ch;
  while( (ch = *s++) != '\0' ) {
    int n;

    if( 'a' <= ch ) {
      n = ch - 'a' + 10;
    } else
    if( 'A' <= ch ) {
      n = ch - 'A' + 10;
    } else
    if( '0' <= ch && ch <= '9' ) {
      n = ch - '0';
    } else {
      break;
    }
    if( n >= base ) break;

    ret = ret * base + n;
  }

  if( sign ) ret = -ret;

  return ret;
}


//================================================================
/*! string copy

  @param  dest		destination buffer.
  @param  destsize	buffer size.
  @param  src		source.
  @return int		number of bytes copied.
*/
int mrbc_strcpy( char *dest, int destsize, const char *src )
{
  int n = destsize;
  if( n <= 0 ) return 0;

  while( --n != 0 ) {
    if( (*dest++ = *src++) == 0 ) goto RETURN;
  }
  *dest = 0;

 RETURN:
  return destsize - n - 1;
}


//================================================================
/*! (beta) Get a N'th argument pointer.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @return	pointer to mrbc_value.

  @remarks
  There is a useful macro MRBC_ARG().\n
*/
mrbc_value * mrbc_arg(struct VM *vm, mrbc_value v[], int argc, int n)
{
  if( argc < n ) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
	"wrong number of arguments (given %d, expected %d)", argc, n);
    return 0;
  }

  return &v[n];
}


//================================================================
/*! (beta) Get a argument as a C integer.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @return	integer value.

  @remarks
  There is a useful macro MRBC_ARG_I().
*/
mrbc_int_t mrbc_arg_i(struct VM *vm, mrbc_value v[], int argc, int n)
{
  if( argc < n ) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
	"wrong number of arguments (given %d, expected %d)", argc, n);
    return 0;
  }

  switch(v[n].tt) {
  case MRBC_TT_INTEGER:
    return v[n].i;

  case MRBC_TT_FLOAT:
    return v[n].d;

  default:
    ;
  }

  mrbc_raisef(vm, MRBC_CLASS(TypeError), "argument %d must be Integer or Float", n);
  return 0;
}


//================================================================
/*! (beta) Get a argument as a C integer with default value.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @param  default_value default value.
  @return	integer value.

  @remarks
  There is a useful macro MRBC_ARG_I().
*/
mrbc_int_t mrbc_arg_i2(struct VM *vm, mrbc_value v[], int argc, int n, mrbc_int_t default_value)
{
  if( argc < n ) return default_value;

  return mrbc_arg_i( vm, v, argc, n );
}


//================================================================
/*! (beta) Get a argument as a C float (double).

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @return	float value.

  @remarks
  There is a useful macro MRBC_ARG_F().
*/
mrbc_float_t mrbc_arg_f(struct VM *vm, mrbc_value v[], int argc, int n)
{
  if( argc < n ) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
	"wrong number of arguments (given %d, expected %d)", argc, n);
    return 0;
  }

  switch(v[n].tt) {
  case MRBC_TT_INTEGER:
    return v[n].i;

  case MRBC_TT_FLOAT:
    return v[n].d;

  default:
    ;
  }

  mrbc_raisef(vm, MRBC_CLASS(TypeError), "argument %d must be Integer or Float", n);
  return 0;
}


//================================================================
/*! (beta) Get a argument as a C float (double) with default value.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @param  default_value default value.
  @return	float value.

  @remarks
  There is a useful macro MRBC_ARG_F().
*/
mrbc_float_t mrbc_arg_f2(struct VM *vm, mrbc_value v[], int argc, int n, mrbc_float_t default_value)
{
  if( argc < n ) return default_value;

  return mrbc_arg_f( vm, v, argc, n );
}


//================================================================
/*! (beta) Get a argument as a C string.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @return	pointer to C string.

  @remarks
  There is a useful macro MRBC_ARG_S().\n
  This function changes the n'th argument type to String.
*/
const char * mrbc_arg_s(struct VM *vm, mrbc_value v[], int argc, int n)
{
  if( argc < n ) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
	"wrong number of arguments (given %d, expected %d)", argc, n);
    return 0;
  }

  switch(v[n].tt) {
  case MRBC_TT_STRING:
    return mrbc_string_cstr( &v[n] );

  default:
    ;
  }

  mrbc_raisef(vm, MRBC_CLASS(TypeError), "argument %d must be String", n);
  return 0;
}


//================================================================
/*! (beta) Get a argument as a C string with default value.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @param  default_value default value.
  @return	pointer to C string.

  @remarks
  There is a useful macro MRBC_ARG_S().\n
  This function changes the n'th argument type to String.
*/
const char * mrbc_arg_s2(struct VM *vm, mrbc_value v[], int argc, int n, const char *default_value)
{
  if( argc < n ) return default_value;

  return mrbc_arg_s( vm, v, argc, n );
}


//================================================================
/*! (beta) Get a True/False argument as a C integer.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @return	bool (0/1) value.

  @remarks
  There is a useful macro MRBC_ARG_B().
*/
int mrbc_arg_b(struct VM *vm, mrbc_value v[], int argc, int n)
{
  if( argc < n ) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
	"wrong number of arguments (given %d, expected %d)", argc, n);
    return 0;
  }

  switch(v[n].tt) {
  case MRBC_TT_FALSE:
    return 0;

  case MRBC_TT_TRUE:
    return 1;

  default:
    ;
  }

  mrbc_raisef(vm, MRBC_CLASS(TypeError), "argument %d must be true or false", n);
  return 0;
}


//================================================================
/*! (beta) Get a True/False argument as a C integer with default value.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  n	target argument number.
  @param  default_value default value.
  @return	bool (0/1) value.

  @remarks
  There is a useful macro MRBC_ARG_B().
*/
int mrbc_arg_b2(struct VM *vm, mrbc_value v[], int argc, int n, int default_value)
{
  if( argc < n ) return default_value;

  return mrbc_arg_b( vm, v, argc, n );
}


//================================================================
/*! (beta) Convert mrbc_value type to Integer.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  val	target value.
  @return	integer value.

  @remarks
  There is a useful macro MRBC_TO_I().
*/
mrbc_int_t mrbc_to_i(struct VM *vm, mrbc_value v[], int argc, mrbc_value *val)
{
  if( val == NULL ) return 0;

  switch(val->tt) {
  case MRBC_TT_EMPTY:
    mrbc_raise(vm, MRBC_CLASS(TypeError), 0);
    return 0;

  case MRBC_TT_INTEGER:
    break;

  case MRBC_TT_FLOAT:
    mrbc_set_integer(val, val->d);
    break;

  default:{
    mrbc_value ret = mrbc_send( vm, v, argc, val, "to_i", 0 );
    if( mrbc_israised(vm) ) return 0;

    mrbc_decref( val );
    *val = ret;
   } break;
  }

  return val->i;
}


//================================================================
/*! (beta) Convert mrbc_value type to Float.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  val	target value.
  @return	float value.

  @remarks
  There is a useful macro MRBC_TO_F().
*/
mrbc_float_t mrbc_to_f(struct VM *vm, mrbc_value v[], int argc, mrbc_value *val)
{
  if( val == NULL ) return 0;

  switch(val->tt) {
  case MRBC_TT_EMPTY:
    mrbc_raise(vm, MRBC_CLASS(TypeError), 0);
    return 0;

  case MRBC_TT_INTEGER:
    mrbc_set_float(val, val->i);
    break;

  case MRBC_TT_FLOAT:
    break;

  default:{
    mrbc_value ret = mrbc_send( vm, v, argc, val, "to_f", 0 );
    if( mrbc_israised(vm) ) return 0;

    mrbc_decref( val );
    *val = ret;
   } break;
  }

  return val->d;
}


//================================================================
/*! (beta) Convert mrbc_value type to String.

  @param  vm	pointer to vm.
  @param  v	argument array.
  @param  argc	num of argument.
  @param  val	target value.
  @return	pointer to C string or NULL.

  @remarks
  There is a useful macro MRBC_TO_S().
*/
char * mrbc_to_s(struct VM *vm, mrbc_value v[], int argc, mrbc_value *val)
{
  if( val == NULL ) return 0;

  switch(val->tt) {
  case MRBC_TT_EMPTY:
    mrbc_raise(vm, MRBC_CLASS(TypeError), 0);
    return 0;

  case MRBC_TT_STRING:
    goto RETURN;

  default:{
    mrbc_value ret = mrbc_send( vm, v, argc, val, "to_s", 0 );
    if( mrbc_israised(vm) ) return 0;

    mrbc_decref( val );
    *val = ret;
   } break;
  }

 RETURN:
  return mrbc_string_cstr( val );
}

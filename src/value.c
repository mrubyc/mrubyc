/*! @file
  @brief
  mruby/c value definitions

  <pre>
  Copyright (C) 2015-2020 Kyushu Institute of Technology.
  Copyright (C) 2015-2020 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/


/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "vm_config.h"
#include <string.h>
#include <assert.h>

/***** Local headers ********************************************************/
#include "value.h"
#include "symbol.h"
#include "class.h"
#include "c_string.h"
#include "c_range.h"
#include "c_array.h"
#include "c_hash.h"


/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
//================================================================
/*! function table for object delete.

  (note) need same order as mrb_vtype.
*/
void (* const mrbc_delfunc[])(mrbc_value *) = {
  mrbc_instance_delete,
  mrbc_proc_delete,
  mrbc_array_delete,
#if MRBC_USE_STRING
  mrbc_string_delete,
#else
  NULL,
#endif
  mrbc_range_delete,
  mrbc_hash_delete,
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
  mrbc_float d1, d2;
#endif

  // if TT_XXX is different
  if( mrbc_type(*v1) != mrbc_type(*v2) ) {
#if MRBC_USE_FLOAT
    // but Numeric?
    if( mrbc_type(*v1) == MRBC_TT_FIXNUM && mrbc_type(*v2) == MRBC_TT_FLOAT ) {
      d1 = v1->i;
      d2 = v2->d;
      goto CMP_FLOAT;
    }
    if( mrbc_type(*v1) == MRBC_TT_FLOAT && mrbc_type(*v2) == MRBC_TT_FIXNUM ) {
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

  case MRBC_TT_FIXNUM:
    return mrbc_fixnum(*v1) - mrbc_fixnum(*v2);

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
  case MRBC_TT_OBJECT:
  case MRBC_TT_PROC:
    return -1 + (v1->proc == v2->proc) + (v1->proc > v2->proc)*2;

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


//================================================================
/*! clear vm id

  @param   v     Pointer to target mrbc_value
*/
void mrbc_clear_vm_id(mrbc_value *v)
{
  switch( mrbc_type(*v) ) {
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


//================================================================
/*! convert ASCII string to integer mruby/c version

  @param  s	source string.
  @param  base	n base.
  @return	result.
*/
mrbc_int mrbc_atoi( const char *s, int base )
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

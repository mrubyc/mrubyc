/*! @file
  @brief
  mruby/c value definitions

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#include "vm_config.h"
#include <string.h>
#include <assert.h>

#include "value.h"
#include "vm.h"
#include "alloc.h"
#include "c_string.h"
#include "c_range.h"
#include "c_array.h"
#include "c_hash.h"


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
  mrbc_float d1, d2;

  // if TT_XXX is different
  if( v1->tt != v2->tt ) {
#if MRBC_USE_FLOAT
    // but Numeric?
    if( v1->tt == MRBC_TT_FIXNUM && v2->tt == MRBC_TT_FLOAT ) {
      d1 = v1->i;
      d2 = v2->d;
      goto CMP_FLOAT;
    }
    if( v1->tt == MRBC_TT_FLOAT && v2->tt == MRBC_TT_FIXNUM ) {
      d1 = v1->d;
      d2 = v2->i;
      goto CMP_FLOAT;
    }
#endif

    // leak Empty?
    if((v1->tt == MRBC_TT_EMPTY && v2->tt == MRBC_TT_NIL) ||
       (v1->tt == MRBC_TT_NIL   && v2->tt == MRBC_TT_EMPTY)) return 0;

    // other case
    return v1->tt - v2->tt;
  }

  // check value
  switch( v1->tt ) {
  case MRBC_TT_NIL:
  case MRBC_TT_FALSE:
  case MRBC_TT_TRUE:
    return 0;

  case MRBC_TT_FIXNUM:
  case MRBC_TT_SYMBOL:
    return v1->i - v2->i;

#if MRBC_USE_FLOAT
  case MRBC_TT_FLOAT:
    d1 = v1->d;
    d2 = v2->d;
    goto CMP_FLOAT;
#endif

  case MRBC_TT_CLASS:
  case MRBC_TT_OBJECT:
  case MRBC_TT_PROC:
    return -1 + (v1->handle == v2->handle) + (v1->handle > v2->handle)*2;

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
/*! Duplicate mrbc_value

  @param   v     Pointer to mrbc_value
*/
void mrbc_dup(mrbc_value *v)
{
  switch( v->tt ){
  case MRBC_TT_OBJECT:
  case MRBC_TT_PROC:
  case MRBC_TT_ARRAY:
  case MRBC_TT_STRING:
  case MRBC_TT_RANGE:
  case MRBC_TT_HASH:
    assert( v->instance->ref_count > 0 );
    assert( v->instance->ref_count != 0xff );	// check max value.
    v->instance->ref_count++;
    break;

  default:
    // Nothing
    break;
  }
}


//================================================================
/*!@brief
  Release object related memory

  @param   v     Pointer to target mrbc_value
*/
void mrbc_release(mrbc_value *v)
{
  mrbc_dec_ref_counter(v);
  v->tt = MRBC_TT_EMPTY;
}


//================================================================
/*!@brief
  Decrement reference counter

  @param   v     Pointer to target mrbc_value
*/
void mrbc_dec_ref_counter(mrbc_value *v)
{
  switch( v->tt ){
  case MRBC_TT_OBJECT:
  case MRBC_TT_PROC:
  case MRBC_TT_ARRAY:
  case MRBC_TT_STRING:
  case MRBC_TT_RANGE:
  case MRBC_TT_HASH:
    assert( v->instance->ref_count != 0 );
    v->instance->ref_count--;
    break;

  default:
    // Nothing
    return;
  }

  // release memory?
  if( v->instance->ref_count != 0 ) return;

  switch( v->tt ) {
  case MRBC_TT_OBJECT:	mrbc_instance_delete(v);	break;
  case MRBC_TT_PROC:	mrbc_raw_free(v->handle);	break;
  case MRBC_TT_ARRAY:	mrbc_array_delete(v);		break;
#if MRBC_USE_STRING
  case MRBC_TT_STRING:	mrbc_string_delete(v);		break;
#endif
  case MRBC_TT_RANGE:	mrbc_range_delete(v);		break;
  case MRBC_TT_HASH:	mrbc_hash_delete(v);		break;

  default:
    // Nothing
    break;
  }
}


//================================================================
/*!@brief
  clear vm id

  @param   v     Pointer to target mrbc_value
*/
void mrbc_clear_vm_id(mrbc_value *v)
{
  switch( v->tt ) {
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
/*!@brief

  convert ASCII string to integer mruby/c version

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

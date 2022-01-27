/*! @file
  @brief
  mruby/c Array class

  <pre>
  Copyright (C) 2015-2020 Kyushu Institute of Technology.
  Copyright (C) 2015-2020 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#include "vm_config.h"
#include <string.h>
#include <assert.h>

#include "value.h"
#include "vm.h"
#include "alloc.h"
#include "class.h"
#include "c_array.h"
#include "c_string.h"
#include "console.h"
#include "opcode.h"

/*
  function summary

 (constructor)
    mrbc_array_new

 (destructor)
    mrbc_array_delete

 (setter)
  --[name]-------------[arg]---[ret]-------------------------------------------
    mrbc_array_set	*T	int
    mrbc_array_push	*T	int
    mrbc_array_unshift	*T	int
    mrbc_array_insert	*T	int

 (getter)
  --[name]-------------[arg]---[ret]---[note]----------------------------------
    mrbc_array_get		T	Data remains in the container
    mrbc_array_pop		T	Data does not remain in the container
    mrbc_array_shift		T	Data does not remain in the container
    mrbc_array_remove		T	Data does not remain in the container

 (others)
    mrbc_array_resize
    mrbc_array_clear
    mrbc_array_compare
    mrbc_array_minmax
    mrbc_array_dup
*/


//================================================================
/*! constructor

  @param  vm	pointer to VM.
  @param  size	initial size
  @return 	array object
*/
mrbc_value mrbc_array_new(struct VM *vm, int size)
{
  mrbc_value value = {.tt = MRBC_TT_ARRAY};

  /*
    Allocate handle and data buffer.
  */
  mrbc_array *h = mrbc_alloc(vm, sizeof(mrbc_array));
  if( !h ) return value;	// ENOMEM

  mrbc_value *data = mrbc_alloc(vm, sizeof(mrbc_value) * size);
  if( !data ) {			// ENOMEM
    mrbc_raw_free( h );
    return value;
  }

  MRBC_INIT_OBJECT_HEADER( h, "AR" );
  h->data_size = size;
  h->n_stored = 0;
  h->data = data;

  value.array = h;
  return value;
}


//================================================================
/*! destructor

  @param  ary	pointer to target value
*/
void mrbc_array_delete(mrbc_value *ary)
{
  mrbc_array *h = ary->array;

  mrbc_value *p1 = h->data;
  const mrbc_value *p2 = p1 + h->n_stored;
  while( p1 < p2 ) {
    mrbc_decref(p1++);
  }

  mrbc_array_delete_handle(ary);
}


//================================================================
/*! clear vm_id

  @param  ary	pointer to target value
*/
void mrbc_array_clear_vm_id(mrbc_value *ary)
{
  mrbc_array *h = ary->array;

  mrbc_set_vm_id( h, 0 );

  mrbc_value *p1 = h->data;
  const mrbc_value *p2 = p1 + h->n_stored;
  while( p1 < p2 ) {
    mrbc_clear_vm_id(p1++);
  }
}


//================================================================
/*! resize buffer

  @param  ary	pointer to target value
  @param  size	size
  @return	mrbc_error_code
*/
int mrbc_array_resize(mrbc_value *ary, int size)
{
  mrbc_array *h = ary->array;

  mrbc_value *data2 = mrbc_raw_realloc(h->data, sizeof(mrbc_value) * size);
  if( !data2 ) return E_NOMEMORY_ERROR;	// ENOMEM

  h->data = data2;
  h->data_size = size;

  return 0;
}


//================================================================
/*! setter

  @param  ary		pointer to target value
  @param  idx		index
  @param  set_val	set value
  @return		mrbc_error_code
*/
int mrbc_array_set(mrbc_value *ary, int idx, mrbc_value *set_val)
{
  mrbc_array *h = ary->array;

  if( idx < 0 ) {
    idx = h->n_stored + idx;
    if( idx < 0 ) return E_INDEX_ERROR;		// raise?
  }

  // need resize?
  if( idx >= h->data_size && mrbc_array_resize(ary, idx + 1) != 0 ) {
    return E_NOMEMORY_ERROR;			// ENOMEM
  }

  if( idx < h->n_stored ) {
    // release existing data.
    mrbc_decref( &h->data[idx] );
  } else {
    // clear empty cells.
    int i;
    for( i = h->n_stored; i < idx; i++ ) {
      h->data[i] = mrbc_nil_value();
    }
    h->n_stored = idx + 1;
  }

  h->data[idx] = *set_val;

  return 0;
}


//================================================================
/*! getter

  @param  ary		pointer to target value
  @param  idx		index
  @return		mrbc_value data at index position or Nil.
*/
mrbc_value mrbc_array_get(const mrbc_value *ary, int idx)
{
  mrbc_array *h = ary->array;

  if( idx < 0 ) idx = h->n_stored + idx;
  if( idx < 0 || idx >= h->n_stored ) return mrbc_nil_value();

  return h->data[idx];
}


//================================================================
/*! push a data to tail

  @param  ary		pointer to target value
  @param  set_val	set value
  @return		mrbc_error_code
*/
int mrbc_array_push(mrbc_value *ary, mrbc_value *set_val)
{
  mrbc_array *h = ary->array;

  if( h->n_stored >= h->data_size ) {
    int size = h->data_size + 6;
    if( mrbc_array_resize(ary, size) != 0 )
      return E_NOMEMORY_ERROR;		// ENOMEM
  }

  h->data[h->n_stored++] = *set_val;

  return 0;
}


//================================================================
/*! pop a data from tail.

  @param  ary		pointer to target value
  @return		tail data or Nil
*/
mrbc_value mrbc_array_pop(struct VM *vm, mrbc_value *ary, uint16_t len)
{
  mrbc_array *h = ary->array;

  if ( len == 1 ){
    if( h->n_stored <= 0 ) return mrbc_nil_value();
    return h->data[--h->n_stored];

  } else if ( len < 0 ) {
    return mrbc_nil_value(); // raise ArgumentError?
  
  } else {

    if( len > h->n_stored ) len = h->n_stored;
    
    mrbc_value arr = mrbc_array_new(vm, len);
    mrbc_value *p = arr.array->data; 

    uint16_t start = h->n_stored - len;
    memmove(p, &h->data[start], sizeof(mrbc_value) * len );
    
    h->n_stored -= len;
    arr.array->n_stored = len;
    return arr;
  }
}


//================================================================
/*! insert a data to the first.

  @param  ary		pointer to target value
  @param  set_val	set value
  @return		mrbc_error_code
*/
int mrbc_array_unshift(mrbc_value *ary, mrbc_value *set_val)
{
  return mrbc_array_insert(ary, 0, set_val);
}


//================================================================
/*! removes the first data and returns it.

  @param  ary		pointer to target value
  @return		first data or Nil
*/
mrbc_value mrbc_array_shift(struct VM *vm, mrbc_value *ary, uint16_t len)
{
  mrbc_array *h = ary->array;

  if ( len == 1 ) {
    if( h->n_stored <= 0 ) return mrbc_nil_value();

    mrbc_value ret = h->data[0];
    memmove(h->data, h->data+1, sizeof(mrbc_value) * --h->n_stored);
    return ret;

  } else if ( len < 0 ) {
    return mrbc_nil_value(); // raise ArgumentError?
  
  } else {

    if( len > h->n_stored ) len = h->n_stored;

    mrbc_value arr = mrbc_array_new(vm, len); 
    memmove(arr.array->data, h->data, sizeof(mrbc_value) * len );
    memmove(h->data, h->data+len, sizeof(mrbc_value) * len );

    h->n_stored -= len;
    arr.array->n_stored = len;
    return arr;
  }
}


//================================================================
/*! insert a data

  @param  ary		pointer to target value
  @param  idx		index
  @param  set_val	set value
  @return		mrbc_error_code
*/
int mrbc_array_insert(mrbc_value *ary, int idx, mrbc_value *set_val)
{
  mrbc_array *h = ary->array;

  if( idx < 0 ) {
    idx = h->n_stored + idx + 1;
    if( idx < 0 ) return E_INDEX_ERROR;		// raise?
  }

  // need resize?
  int size = 0;
  if( idx >= h->data_size ) {
    size = idx + 1;
  } else if( h->n_stored >= h->data_size ) {
    size = h->data_size + 1;
  }
  if( size && mrbc_array_resize(ary, size) != 0 ) {
    return E_NOMEMORY_ERROR;			// ENOMEM
  }

  // move datas.
  if( idx < h->n_stored ) {
    memmove(h->data + idx + 1, h->data + idx,
	    sizeof(mrbc_value) * (h->n_stored - idx));
  }

  // set data
  h->data[idx] = *set_val;
  h->n_stored++;

  // clear empty cells if need.
  if( idx >= h->n_stored ) {
    int i;
    for( i = h->n_stored-1; i < idx; i++ ) {
      h->data[i] = mrbc_nil_value();
    }
    h->n_stored = idx + 1;
  }

  return 0;
}


//================================================================
/*! remove a data

  @param  ary		pointer to target value
  @param  idx		index
  @return		mrbc_value data at index position or Nil.
*/
mrbc_value mrbc_array_remove(mrbc_value *ary, int idx)
{
  mrbc_array *h = ary->array;

  if( idx < 0 ) idx = h->n_stored + idx;
  if( idx < 0 || idx >= h->n_stored ) return mrbc_nil_value();

  mrbc_value val = h->data[idx];
  h->n_stored--;
  if( idx < h->n_stored ) {
    memmove(h->data + idx, h->data + idx + 1,
	    sizeof(mrbc_value) * (h->n_stored - idx));
  }

  return val;
}


//================================================================
/*! clear all

  @param  ary		pointer to target value
*/
void mrbc_array_clear(mrbc_value *ary)
{
  mrbc_array *h = ary->array;

  mrbc_value *p1 = h->data;
  const mrbc_value *p2 = p1 + h->n_stored;
  while( p1 < p2 ) {
    mrbc_decref(p1++);
  }

  h->n_stored = 0;
}


//================================================================
/*! compare

  @param  v1	Pointer to mrbc_value
  @param  v2	Pointer to another mrbc_value
  @retval 0	v1 == v2
  @retval plus	v1 >  v2
  @retval minus	v1 <  v2
*/
int mrbc_array_compare(const mrbc_value *v1, const mrbc_value *v2)
{
  int i;
  for( i = 0; ; i++ ) {
    if( i >= mrbc_array_size(v1) || i >= mrbc_array_size(v2) ) {
      return mrbc_array_size(v1) - mrbc_array_size(v2);
    }

    int res = mrbc_compare( &v1->array->data[i], &v2->array->data[i] );
    if( res != 0 ) return res;
  }
}


//================================================================
/*! get min, max value

  @param  ary		pointer to target value
  @param  pp_min_value	returns minimum mrbc_value
  @param  pp_max_value	returns maxmum mrbc_value
*/
void mrbc_array_minmax(mrbc_value *ary, mrbc_value **pp_min_value, mrbc_value **pp_max_value)
{
  mrbc_array *h = ary->array;

  if( h->n_stored == 0 ) {
    *pp_min_value = NULL;
    *pp_max_value = NULL;
    return;
  }

  mrbc_value *p_min_value = h->data;
  mrbc_value *p_max_value = h->data;

  int i;
  for( i = 1; i < h->n_stored; i++ ) {
    if( mrbc_compare( &h->data[i], p_min_value ) < 0 ) {
      p_min_value = &h->data[i];
    }
    if( mrbc_compare( &h->data[i], p_max_value ) > 0 ) {
      p_max_value = &h->data[i];
    }
  }

  *pp_min_value = p_min_value;
  *pp_max_value = p_max_value;
}


//================================================================
/*! duplicate (shallow copy)

  @param  vm	pointer to VM.
  @param  ary	source
  @return	result
*/
mrbc_value mrbc_array_dup(struct VM *vm, const mrbc_value *ary)
{
  mrbc_array *sh = ary->array;

  mrbc_value dv = mrbc_array_new(vm, sh->n_stored);
  if( dv.array == NULL ) return dv;		// ENOMEM

  memcpy( dv.array->data, sh->data, sizeof(mrbc_value) * sh->n_stored );
  dv.array->n_stored = sh->n_stored;

  mrbc_value *p1 = dv.array->data;
  const mrbc_value *p2 = p1 + dv.array->n_stored;
  while( p1 < p2 ) {
    mrbc_incref(p1++);
  }

  return dv;
}


//================================================================
/*! method new
*/
static void c_array_new(struct VM *vm, mrbc_value v[], int argc)
{
  /*
    in case of new()
  */
  if( argc == 0 ) {
    mrbc_value ret = mrbc_array_new(vm, 0);
    if( ret.array == NULL ) return;		// ENOMEM

    SET_RETURN(ret);
    return;
  }

  /*
    in case of new(num)
  */
  if( argc == 1 && v[1].tt == MRBC_TT_FIXNUM && v[1].i >= 0 ) {
    mrbc_value ret = mrbc_array_new(vm, v[1].i);
    if( ret.array == NULL ) return;		// ENOMEM

    mrbc_value nil = mrbc_nil_value();
    if( v[1].i > 0 ) {
      mrbc_array_set(&ret, v[1].i - 1, &nil);
    }
    SET_RETURN(ret);
    return;
  }

  /*
    in case of new(num, value)
  */
  if( argc == 2 && v[1].tt == MRBC_TT_FIXNUM && v[1].i >= 0 ) {
    mrbc_value ret = mrbc_array_new(vm, v[1].i);
    if( ret.array == NULL ) return;		// ENOMEM

    int i;
    for( i = 0; i < v[1].i; i++ ) {
      mrbc_incref(&v[2]);
      mrbc_array_set(&ret, i, &v[2]);
    }
    SET_RETURN(ret);
    return;
  }

  /*
    other case
  */
  console_print( "ArgumentError\n" );	// raise?
}


//================================================================
/*! (operator) +
*/
static void c_array_add(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrb_type(v[1]) != MRBC_TT_ARRAY ) {
    console_print( "TypeError\n" );	// raise?
    return;
  }

  mrbc_array *h1 = v[0].array;
  mrbc_array *h2 = v[1].array;

  mrbc_value value = mrbc_array_new(vm, h1->n_stored + h2->n_stored);
  if( value.array == NULL ) return;		// ENOMEM

  memcpy( value.array->data,                h1->data,
	  sizeof(mrbc_value) * h1->n_stored );
  memcpy( value.array->data + h1->n_stored, h2->data,
	  sizeof(mrbc_value) * h2->n_stored );
  value.array->n_stored = h1->n_stored + h2->n_stored;

  mrbc_value *p1 = value.array->data;
  const mrbc_value *p2 = p1 + value.array->n_stored;
  while( p1 < p2 ) {
    mrbc_incref(p1++);
  }

  mrbc_decref_empty(v+1);
  SET_RETURN(value);
}


//================================================================
/*! (operator) []
*/
static void c_array_get(struct VM *vm, mrbc_value v[], int argc)
{
  /*
    in case of Array[...] -> Array
  */
  if( v[0].tt == MRBC_TT_CLASS ) {
    mrbc_value ret = mrbc_array_new(vm, argc);
    if( ret.array == NULL ) return;	// ENOMEM

    memcpy( ret.array->data, &v[1], sizeof(mrbc_value) * argc );
    int i;
    for( i = 1; i <= argc; i++ ) {
      v[i].tt = MRBC_TT_EMPTY;
    }
    ret.array->n_stored = argc;

    SET_RETURN(ret);
    return;
  }

  /*
    in case of self[nth] -> object | nil
  */
  if( argc == 1 && v[1].tt == MRBC_TT_FIXNUM ) {
    mrbc_value ret = mrbc_array_get(v, v[1].i);
    mrbc_incref(&ret);
    SET_RETURN(ret);
    return;
  }

  /*
    in case of self[start, length] -> Array | nil
  */
  if( argc == 2 && v[1].tt == MRBC_TT_FIXNUM && v[2].tt == MRBC_TT_FIXNUM ) {
    int len = mrbc_array_size(&v[0]);
    int idx = v[1].i;
    if( idx < 0 ) idx += len;
    if( idx < 0 ) goto RETURN_NIL;

    int size = (v[2].i < (len - idx)) ? v[2].i : (len - idx);
					// min( v[2].i, (len - idx) )
    if( size < 0 ) goto RETURN_NIL;

    mrbc_value ret = mrbc_array_new(vm, size);
    if( ret.array == NULL ) return;		// ENOMEM

    int i;
    for( i = 0; i < size; i++ ) {
      mrbc_value val = mrbc_array_get(v, v[1].i + i);
      mrbc_incref(&val);
      mrbc_array_push(&ret, &val);
    }

    SET_RETURN(ret);
    return;
  }

  /*
    other case
  */
  console_print( "Not support such case in Array#[].\n" );
  return;

 RETURN_NIL:
  SET_NIL_RETURN();
}


//================================================================
/*! (operator) []=
*/
static void c_array_set(struct VM *vm, mrbc_value v[], int argc)
{
  /*
    in case of self[nth] = val
  */
  if( argc == 2 && v[1].tt == MRBC_TT_FIXNUM ) {
    mrbc_array_set(v, v[1].i, &v[2]);	// raise? IndexError or ENOMEM
    v[2].tt = MRBC_TT_EMPTY;
    return;
  }

  /*
    in case of self[start, length] = val
  */
  if( argc == 3 && v[1].tt == MRBC_TT_FIXNUM && v[2].tt == MRBC_TT_FIXNUM ) {

    uint16_t size = mrbc_array_size(v);
    mrbc_int start = v[1].i;
    mrbc_int len = v[2].i;

    // case 1: start is larger than size
    // just ignore length
    if ( start >= size ) {
      mrbc_array_set(v, start, &v[3]);	// raise? IndexError or ENOMEM
      v[3].tt = MRBC_TT_EMPTY;
      return;
    }

    // case 2: start is smaller than size, length is 1
    // just replace
    if ( start < size && len == 1 ) {
      mrbc_array_set(v, start, &v[3]);	// raise? IndexError or ENOMEM
      v[3].tt = MRBC_TT_EMPTY;
      return;
    }

    // case 3: start is smaller than size, length is 0
    // just insert
    if ( start < size && len == 0 ) {
      mrbc_array_insert(v, start, &v[3]);	// raise? IndexError or ENOMEM
      v[3].tt = MRBC_TT_EMPTY;
      return;
    } 
    
    // case 4: start is smaller than size, length is >= size-start
    if ( start < size && len >= size-start ) {
      mrbc_array_set(v, start, &v[3]);	// raise? IndexError or ENOMEM
      v->array->n_stored = start + 1;

      v[3].tt = MRBC_TT_EMPTY;
      return;

    } else {
    // case 5: start is smaller than index, length is < size-start
      mrbc_array_set(v, start, &v[3]);	// raise? IndexError or ENOMEM
      mrbc_value *p = v->array->data;
      memmove(p+start+1, p+start+len, sizeof(mrbc_value) * (size-start-len));
      v->array->n_stored -= len-1;

      v[3].tt = MRBC_TT_EMPTY;
      return;
    }

  }

  /*
    other case
  */
  console_print( "Not support such case in Array#[].\n" );
}


//================================================================
/*! (method) clear
*/
static void c_array_clear(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_array_clear(v);
}


//================================================================
/*! (method) delete_at
*/
static void c_array_delete_at(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value val = mrbc_array_remove(v, GET_INT_ARG(1));
  SET_RETURN(val);
}


//================================================================
/*! (method) empty?
*/
static void c_array_empty(struct VM *vm, mrbc_value v[], int argc)
{
  int n = mrbc_array_size(v);

  if( n ) {
    SET_FALSE_RETURN();
  } else {
    SET_TRUE_RETURN();
  }
}


//================================================================
/*! (method) size,length,count
*/
static void c_array_size(struct VM *vm, mrbc_value v[], int argc)
{
  int n = mrbc_array_size(v);

  SET_INT_RETURN(n);
}


//================================================================
/*! (method) index
*/
static void c_array_index(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value *value = &GET_ARG(1);
  mrbc_value *data = v->array->data;
  int n = v->array->n_stored;
  int i;

  for( i = 0; i < n; i++ ) {
    if( mrbc_compare(&data[i], value) == 0 ) break;
  }

  if( i < n ) {
    SET_INT_RETURN(i);
  } else {
    SET_NIL_RETURN();
  }
}


//================================================================
/*! (method) include?
*/
static void c_array_include(struct VM *vm, mrbc_value v[], int argc)
{
  c_array_index(vm, v, argc);

  SET_BOOL_RETURN( mrb_type(v[0]) == MRBC_TT_FIXNUM );
}


//================================================================
/*! (method) first
*/
static void c_array_first(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value val = mrbc_array_get(v, 0);
  mrbc_incref(&val);
  SET_RETURN(val);
}


//================================================================
/*! (method) last
*/
static void c_array_last(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value val = mrbc_array_get(v, -1);
  mrbc_incref(&val);
  SET_RETURN(val);
}


//================================================================
/*! (method) push
*/
static void c_array_push(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_array_push(&v[0], &v[1]);	// raise? ENOMEM
  v[1].tt = MRBC_TT_EMPTY;
}


//================================================================
/*! (method) pop
*/
static void c_array_pop(struct VM *vm, mrbc_value v[], int argc)
{
  /*
    in case of pop() -> object | nil
  */
  if( argc == 0 ) {
    mrbc_value val = mrbc_array_pop(vm, v, 1);
    SET_RETURN(val);
    return;
  }

  /*
    in case of pop(n) -> Array
  */
  if( argc == 1 && v[1].tt == MRBC_TT_FIXNUM ) {
    mrbc_value val = mrbc_array_pop(vm, v, v[1].i);
    SET_RETURN(val);
    return;
  }

  /*
    other case
  */
  console_print( "Not support such case in Array#pop.\n" );
}


//================================================================
/*! (method) unshift
*/
static void c_array_unshift(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_array_unshift(&v[0], &v[1]);	// raise? IndexError or ENOMEM
  v[1].tt = MRBC_TT_EMPTY;
}


//================================================================
/*! (method) shift
*/
static void c_array_shift(struct VM *vm, mrbc_value v[], int argc)
{
  /*
    in case of pop() -> object | nil
  */
  if( argc == 0 ) {
    mrbc_value val = mrbc_array_shift(vm, v, 1);
    SET_RETURN(val);
    return;
  }

  /*
    in case of pop(n) -> Array
  */
  if( argc == 1 && v[1].tt == MRBC_TT_FIXNUM ) {
    mrbc_value val = mrbc_array_shift(vm, v, v[1].i);
    SET_RETURN(val);
    return;
  }

  /*
    other case
  */
  console_print( "Not support such case in Array#shift.\n" );
}


//================================================================
/*! (method) dup
*/
static void c_array_dup(struct VM *vm, mrbc_value v[], int argc)
{
  SET_RETURN( mrbc_array_dup( vm, &v[0] ) );
}


//================================================================
/*! (method) min
*/
static void c_array_min(struct VM *vm, mrbc_value v[], int argc)
{
  // Subset of Array#min, not support min(n).

  mrbc_value *p_min_value, *p_max_value;

  mrbc_array_minmax(&v[0], &p_min_value, &p_max_value);
  if( p_min_value == NULL ) {
    SET_NIL_RETURN();
    return;
  }

  mrbc_incref(p_min_value);
  SET_RETURN(*p_min_value);
}


//================================================================
/*! (method) max
*/
static void c_array_max(struct VM *vm, mrbc_value v[], int argc)
{
  // Subset of Array#max, not support max(n).

  mrbc_value *p_min_value, *p_max_value;

  mrbc_array_minmax(&v[0], &p_min_value, &p_max_value);
  if( p_max_value == NULL ) {
    SET_NIL_RETURN();
    return;
  }

  mrbc_incref(p_max_value);
  SET_RETURN(*p_max_value);
}


//================================================================
/*! (method) minmax
*/
static void c_array_minmax(struct VM *vm, mrbc_value v[], int argc)
{
  // Subset of Array#minmax, not support minmax(n).

  mrbc_value *p_min_value, *p_max_value;
  mrbc_value nil = mrbc_nil_value();
  mrbc_value ret = mrbc_array_new(vm, 2);

  mrbc_array_minmax(&v[0], &p_min_value, &p_max_value);
  if( p_min_value == NULL ) p_min_value = &nil;
  if( p_max_value == NULL ) p_max_value = &nil;

  mrbc_incref(p_min_value);
  mrbc_incref(p_max_value);
  mrbc_array_set(&ret, 0, p_min_value);
  mrbc_array_set(&ret, 1, p_max_value);

  SET_RETURN(ret);
}


#if MRBC_USE_STRING
//================================================================
/*! (method) inspect
*/
static void c_array_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_string_new_cstr(vm, "[");
  if( !ret.string ) goto RETURN_NIL;		// ENOMEM

  int i;
  for( i = 0; i < mrbc_array_size(v); i++ ) {
    if( i != 0 ) mrbc_string_append_cstr( &ret, ", " );

    mrbc_value v1 = mrbc_array_get(v, i);
    mrbc_value s1 = mrbc_send( vm, v, argc, &v1, "inspect", 0 );
    mrbc_string_append( &ret, &s1 );
    mrbc_string_delete( &s1 );
  }

  mrbc_string_append_cstr( &ret, "]" );

  SET_RETURN(ret);
  return;

 RETURN_NIL:
  SET_NIL_RETURN();
}


//================================================================
/*! (method) join
*/
static void c_array_join_1(struct VM *vm, mrbc_value v[], int argc,
			   mrbc_value *src, mrbc_value *ret, mrbc_value *separator)
{
  if( mrbc_array_size(src) == 0 ) return;

  int i = 0;
  int flag_error = 0;
  while( !flag_error ) {
    if( src->array->data[i].tt == MRBC_TT_ARRAY ) {
      c_array_join_1(vm, v, argc, &src->array->data[i], ret, separator);
    } else {
      mrbc_value v1 = mrbc_send( vm, v, argc, &src->array->data[i], "to_s", 0 );
      flag_error |= mrbc_string_append( ret, &v1 );
      mrbc_decref(&v1);
    }
    if( ++i >= mrbc_array_size(src) ) break;	// normal return.
    flag_error |= mrbc_string_append( ret, separator );
  }
}

static void c_array_join(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_string_new(vm, NULL, 0);
  if( !ret.string ) goto RETURN_NIL;		// ENOMEM

  mrbc_value separator = (argc == 0) ? mrbc_string_new_cstr(vm, "") :
    mrbc_send( vm, v, argc, &v[1], "to_s", 0 );

  c_array_join_1(vm, v, argc, &v[0], &ret, &separator );
  mrbc_decref(&separator);

  SET_RETURN(ret);
  return;

 RETURN_NIL:
  SET_NIL_RETURN();
}

#endif


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Array")
  FILE("method_table_array.h")
  FUNC("mrbc_init_class_array")

  METHOD( "new",	c_array_new )
  METHOD( "+",		c_array_add )
  METHOD( "[]",		c_array_get )
  METHOD( "at",		c_array_get )
  METHOD( "[]=",	c_array_set )
  METHOD( "<<",		c_array_push )
  METHOD( "clear",	c_array_clear )
  METHOD( "delete_at",	c_array_delete_at )
  METHOD( "empty?",	c_array_empty )
  METHOD( "size",	c_array_size )
  METHOD( "length",	c_array_size )
  METHOD( "count",	c_array_size )
  METHOD( "index",	c_array_index )
  METHOD( "include?",	c_array_include )
  METHOD( "first",	c_array_first )
  METHOD( "last",	c_array_last )
  METHOD( "push",	c_array_push )
  METHOD( "pop",	c_array_pop )
  METHOD( "shift",	c_array_shift )
  METHOD( "unshift",	c_array_unshift )
  METHOD( "dup",	c_array_dup )
  METHOD( "min",	c_array_min )
  METHOD( "max",	c_array_max )
  METHOD( "minmax",	c_array_minmax )
#if MRBC_USE_STRING
  METHOD( "inspect",	c_array_inspect )
  METHOD( "to_s",	c_array_inspect )
  METHOD( "join",	c_array_join )
#endif
*/
#include "method_table_array.h"

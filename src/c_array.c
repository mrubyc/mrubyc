/*! @file
  @brief
  mruby/c Array class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


 Function summary

 (constructor)
    mrbc_array_new()

 (destructor)
    mrbc_array_delete()

 (setter)
  --[name]-------------[arg]---[ret]-------------------------------------------
    mrbc_array_set()	 *V	int
    mrbc_array_push()	 *V	int
    mrbc_array_push_m()	 *V	int
    mrbc_array_unshift() *V	int
    mrbc_array_insert()	 *V	int

 (getter)
  --[name]-------------[arg]---[ret]---[note]----------------------------------
    mrbc_array_get()	idx	 V	Data remains in the container
    mrbc_array_get_p()	idx	*V	Data remains in the container
    mrbc_array_pop()		 V	Data does not remain in the container
    mrbc_array_shift()		 V	Data does not remain in the container
    mrbc_array_remove()	idx	 V	Data does not remain in the container

 (finder)
    mrbc_array_index()
    mrbc_array_include()

 (others)
    mrbc_array_size()
    mrbc_array_resize()
    mrbc_array_clear()
    mrbc_array_compare()
    mrbc_array_minmax()
    mrbc_array_dup()
    mrbc_array_divide()
    mrbc_array_uniq()
    mrbc_array_uniq_self()

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

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! constructor

  @param  vm	pointer to VM.
  @param  size	initial size
  @return 	array object
*/
mrbc_value mrbc_array_new(struct VM *vm, int size)
{
  mrbc_value value = mrbc_immediate_value(MRBC_TT_ARRAY);

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


#if defined(MRBC_ALLOC_VMID)
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
#endif


//================================================================
/*! resize buffer

  @param  ary	pointer to target value
  @param  size	size
  @return	mrbc_error_code
*/
int mrbc_array_resize(mrbc_value *ary, int size)
{
  if( size <= 0 ) size = 1;

  mrbc_array *h = ary->array;
  mrbc_value *data = mrbc_raw_realloc(h->data, sizeof(mrbc_value) * size);
  if( !data ) return E_NOMEMORY_ERROR;	// ENOMEM

  h->data = data;
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
    if( idx < 0 ) return E_INDEX_ERROR;
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
    for( int i = h->n_stored; i < idx; i++ ) {
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
/*! getter

  @param  ary		pointer to target value
  @param  idx		index
  @return		pointer to mrbc_value or NULL.
*/
mrbc_value * mrbc_array_get_p(const mrbc_value *ary, int idx)
{
  mrbc_array *h = ary->array;

  if( idx < 0 ) idx = h->n_stored + idx;
  if( idx < 0 || idx >= h->n_stored ) return NULL;

  return &h->data[idx];
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
    if( mrbc_array_resize(ary, size) != 0 ) return E_NOMEMORY_ERROR; // ENOMEM
  }

  h->data[h->n_stored++] = *set_val;

  return 0;
}


//================================================================
/*! push multiple data to tail

  @param  ary		pointer to target value
  @param  set_val	set value (array)
  @return		mrbc_error_code
*/
int mrbc_array_push_m(mrbc_value *ary, mrbc_value *set_val)
{
  mrbc_array *ha_d = ary->array;
  mrbc_array *ha_s = set_val->array;
  int new_size = ha_d->n_stored + ha_s->n_stored;

  if( new_size > ha_d->data_size ) {
    if( mrbc_array_resize(ary, new_size) != 0 )
      return E_NOMEMORY_ERROR;		// ENOMEM
  }

  memcpy( &ha_d->data[ha_d->n_stored], ha_s->data,
          sizeof(mrbc_value) * ha_s->n_stored );
  ha_d->n_stored += ha_s->n_stored;

  return 0;
}


//================================================================
/*! pop a data from tail.

  @param  ary		pointer to target value
  @return		tail data or Nil
*/
mrbc_value mrbc_array_pop(mrbc_value *ary)
{
  mrbc_array *h = ary->array;

  if( h->n_stored <= 0 ) return mrbc_nil_value();
  return h->data[--h->n_stored];
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
mrbc_value mrbc_array_shift(mrbc_value *ary)
{
  mrbc_array *h = ary->array;

  if( h->n_stored <= 0 ) return mrbc_nil_value();

  mrbc_value ret = h->data[0];
  memmove(h->data, h->data+1, sizeof(mrbc_value) * --h->n_stored);

  return ret;
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
    if( idx < 0 ) return E_INDEX_ERROR;
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
    for( int i = h->n_stored-1; i < idx; i++ ) {
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

  mrbc_value ret = h->data[idx];
  h->n_stored--;
  if( idx < h->n_stored ) {
    memmove(h->data + idx, h->data + idx + 1,
            sizeof(mrbc_value) * (h->n_stored - idx));
  }

  return ret;
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
  for( int i = 0; ; i++ ) {
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

  for( int i = 1; i < h->n_stored; i++ ) {
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
/*! divide into two parts

  @param  vm	pointer to VM.
  @param  src	source
  @param  pos	divide position
  @return	divided array
  @note
    src = [0,1,2,3]
    ret = divide(src, 2)
    src = [0,1], ret = [2,3]
*/
mrbc_value mrbc_array_divide(struct VM *vm, mrbc_value *src, int pos)
{
  mrbc_array *ha_s = src->array;
  if( pos < 0 ) pos = 0;
  int new_size = ha_s->n_stored - pos;
  if( new_size < 0 ) new_size = 0;
  int remain_size = ha_s->n_stored - new_size;

  mrbc_value ret = mrbc_array_new(vm, new_size);
  if( ret.array == NULL ) return ret;		// ENOMEM
  mrbc_array *ha_r = ret.array;

  memcpy( ha_r->data, ha_s->data + remain_size, sizeof(mrbc_value) * new_size );
  ha_s->n_stored = remain_size;
  mrbc_array_resize( src, remain_size );
  ha_r->n_stored = new_size;

  return ret;
}

//================================================================
/*! index

  @param  ary	target array.
  @param  val	search object.
  @return	index value or -1 if not found.
*/
int mrbc_array_index(const mrbc_value *ary, const mrbc_value *val)
{
  int n = ary->array->n_stored;
  for( int i = 0; i < n; i++ ) {
    if( mrbc_compare(&ary->array->data[i], val) == 0 ) return i;
  }
  return -1;
}


//================================================================
/*! removes duplicate elements and return allocated new mrbc_value

  @param  vm	pointer to VM.
  @param  ary   source
  @return	result
*/
mrbc_value mrbc_array_uniq(struct VM *vm, const mrbc_value *ary)
{
  mrbc_value ret = mrbc_array_dup(vm, ary);

  mrbc_array_uniq_self( &ret );
  return ret;
}


//================================================================
/*! removes duplicate elements

  @param  ary   target
  @return	num of deleted
*/
int mrbc_array_uniq_self(mrbc_value *ary)
{
  mrbc_array *ah = ary->array;
  int size = ah->n_stored;

  for( int i = 0; i < size-1; i++ ) {
    for( int j = i+1; j < size; j++ ) {
      if( mrbc_compare( &ah->data[i], &ah->data[j] ) != 0 ) continue;

      mrbc_decref( &ah->data[j] );
      int rest = --size - j;
      if( rest == 0 ) break;

      memmove( &ah->data[j], &ah->data[j+1], sizeof(mrbc_value) * rest );
      j--;
    }
  }

  int ret = ah->n_stored - size;
  ah->n_stored = size;

  return ret;
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
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_INTEGER && mrbc_integer(v[1]) >= 0 ) {
    int num = mrbc_integer(v[1]);
    mrbc_value ret = mrbc_array_new(vm, num);
    if( ret.array == NULL ) return;		// ENOMEM

    if( num > 0 ) {
      mrbc_array_set(&ret, num - 1, &mrbc_nil_value());
    }
    SET_RETURN(ret);
    return;
  }

  /*
    in case of new(num, value)
  */
  if( argc == 2 && mrbc_type(v[1]) == MRBC_TT_INTEGER && mrbc_integer(v[1]) >= 0 ) {
    int num = mrbc_integer(v[1]);
    mrbc_value ret = mrbc_array_new(vm, num);
    if( ret.array == NULL ) return;		// ENOMEM

    for( int i = 0; i < num; i++ ) {
      mrbc_incref(&v[2]);
      mrbc_array_set(&ret, i, &v[2]);
    }
    SET_RETURN(ret);
    return;
  }

  /*
    other case
  */
  mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
}


//================================================================
/*! (operator) +
*/
static void c_array_add(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[1]) != MRBC_TT_ARRAY ) {
    mrbc_raise( vm, MRBC_CLASS(TypeError), 0 );
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
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    mrbc_value ret = mrbc_array_new(vm, argc);
    if( ret.array == NULL ) return;	// ENOMEM

    memcpy( ret.array->data, &v[1], sizeof(mrbc_value) * argc );
    for( int i = 1; i <= argc; i++ ) {
      mrbc_set_tt( &v[i], MRBC_TT_EMPTY );
    }
    ret.array->n_stored = argc;

    SET_RETURN(ret);
    return;
  }

  /*
    in case of self[nth] -> object | nil
  */
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_INTEGER ) {
    mrbc_value ret = mrbc_array_get(v, mrbc_integer(v[1]));
    mrbc_incref(&ret);
    SET_RETURN(ret);
    return;
  }

  /*
    in case of self[start, length] -> Array | nil
  */
  if( argc == 2 && mrbc_type(v[1]) == MRBC_TT_INTEGER && mrbc_type(v[2]) == MRBC_TT_INTEGER ) {
    int len = mrbc_array_size(&v[0]);
    int idx = mrbc_integer(v[1]);
    if( idx < 0 ) idx += len;
    if( idx < 0 ) goto RETURN_NIL;

    int size = (mrbc_integer(v[2]) < (len - idx)) ? mrbc_integer(v[2]) : (len - idx);
		// min( mrbc_integer(v[2]), (len - idx) )
    if( size < 0 ) goto RETURN_NIL;

    mrbc_value ret = mrbc_array_new(vm, size);
    if( ret.array == NULL ) return;		// ENOMEM

    for( int i = 0; i < size; i++ ) {
      mrbc_value val = mrbc_array_get(v, mrbc_integer(v[1]) + i);
      mrbc_incref(&val);
      mrbc_array_push(&ret, &val);
    }

    SET_RETURN(ret);
    return;
  }

  /*
    other case
  */
  mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
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
  if( argc == 2 && mrbc_type(v[1]) == MRBC_TT_INTEGER ) {
    if( mrbc_array_set(v, mrbc_integer(v[1]), &v[2]) != 0 ) {
      mrbc_raise( vm, MRBC_CLASS(IndexError), "too small for array");
      return;
    }

    // return val
    mrbc_incref(&v[2]);
    mrbc_decref(&v[0]);
    v[0] = v[2];
    mrbc_set_tt(&v[2], MRBC_TT_EMPTY);
    return;
  }

  /*
    in case of self[start, length] = val
  */
  if( argc == 3 && mrbc_type(v[1]) == MRBC_TT_INTEGER &&
                   mrbc_type(v[2]) == MRBC_TT_INTEGER ) {
    int pos = mrbc_integer(v[1]);
    int len = mrbc_integer(v[2]);

    if( pos < 0 ) {
      pos = v[0].array->n_stored + pos;
      if( pos < 0 ) {
        mrbc_raise( vm, MRBC_CLASS(IndexError), "index too small for array");
        return;
      }
    } else if( pos > v[0].array->n_stored ) {
      mrbc_array_set( &v[0], pos-1, &mrbc_nil_value() );
      len = 0;
    }
    if( len < 0 ) {
      mrbc_raise( vm, MRBC_CLASS(IndexError), "negative length");
      return;
    }
    if( pos+len > v[0].array->n_stored ) {
      len = v[0].array->n_stored - pos;
    }

    // split 2 part
    mrbc_value v1 = mrbc_array_divide(vm, &v[0], pos+len);
    mrbc_array *ha0 = v[0].array;

    // delete data from tail.
    for( int i = 0; i < len; i++ ) {
      mrbc_decref( &ha0->data[--ha0->n_stored] );
    }

    // append data
    if( mrbc_type(v[3]) == MRBC_TT_ARRAY ) {
      mrbc_array_push_m(&v[0], &v[3]);
      for( int i = 0; i < v[3].array->n_stored; i++ ) {
        mrbc_incref( &v[3].array->data[i] );
      }
    } else {
      mrbc_incref(&v[3]);
      mrbc_array_push(&v[0], &v[3]);
    }

    mrbc_array_push_m(&v[0], &v1);
    mrbc_array_delete_handle( &v1 );

    // return val
    mrbc_decref(&v[0]);
    v[0] = v[3];
    mrbc_set_tt(&v[3], MRBC_TT_EMPTY);
    return;
  }

  /*
    other case
  */
  mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
}


//================================================================
/*! (method) clear
*/
static void c_array_clear(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_array_clear(v);
}


//================================================================
/*! (method) difference(*other_arrays) -> Array
*/
static void c_array_difference(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_array_dup(vm, &v[0]);

  for( int i = 1; i <= argc; i++ ) {
    if( mrbc_type(v[i]) != MRBC_TT_ARRAY ) {
      mrbc_raise( vm, MRBC_CLASS(TypeError), 0 );
      return;
    }

    for( int j = 0; j < mrbc_array_size(&v[i]); j++ ) {
      int idx;
      while( (idx = mrbc_array_index( &ret, &v[i].array->data[j] )) >= 0 ) {
        mrbc_array *ah = ret.array;
        ah->n_stored--;
        memmove(ah->data + idx, ah->data + idx + 1,
                sizeof(mrbc_value) * (ah->n_stored - idx));
      }
    }
  }

  SET_RETURN(ret);
}


//================================================================
/*! (method) delete_at
*/
static void c_array_delete_at(struct VM *vm, mrbc_value v[], int argc)
{
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_INTEGER ) {
    mrbc_value val = mrbc_array_remove(v, mrbc_integer(v[1]));
    SET_RETURN(val);
  } else {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }
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
/*! (method) include?
*/
static void c_array_include(struct VM *vm, mrbc_value v[], int argc)
{
  SET_BOOL_RETURN(0 < mrbc_array_include(&v[0], &v[1]));
}


//================================================================
/*! (method) &
*/
static void c_array_and(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[1]) != MRBC_TT_ARRAY ) {
    mrbc_raisef( vm, MRBC_CLASS(TypeError), "no implicit conversion into %s", "Array");
    return;
  }
  mrbc_value result = mrbc_array_new(vm, 0);
  for( int i = 0; i < v[0].array->n_stored; i++) {
    mrbc_value *data = &v[0].array->data[i];
    if (0 < mrbc_array_include(&v[1], data) && 0 == mrbc_array_include(&result, data))
    {
      mrbc_array_push(&result, data);
    }
  }
  SET_RETURN(result);
}


//================================================================
/*! (method) |
*/
static void c_array_or(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[1]) != MRBC_TT_ARRAY ) {
    mrbc_raisef( vm, MRBC_CLASS(TypeError), "no implicit conversion into %s", "Array");
    return;
  }
  mrbc_value result = mrbc_array_new(vm, 0);
  for( int i = 0; i < v[0].array->n_stored; i++) {
    mrbc_value *data = &v[0].array->data[i];
    if (0 == mrbc_array_include(&result, data))
    {
      mrbc_array_push(&result, data);
    }
  }

  for( int i = 0; i < v[1].array->n_stored; i++) {
    mrbc_value *data = &v[1].array->data[i];
    if (0 == mrbc_array_include(&result, data))
    {
      mrbc_array_push(&result, data);
    }
  }
  SET_RETURN(result);
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
  mrbc_array_push(&v[0], &v[1]);
  mrbc_set_tt( &v[1], MRBC_TT_EMPTY );
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
    mrbc_value val = mrbc_array_pop(v);
    SET_RETURN(val);
    return;
  }

  /*
    in case of pop(n) -> Array
  */
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_INTEGER ) {
    int pos = mrbc_array_size(&v[0]) - v[1].i;
    mrbc_value val = mrbc_array_divide(vm, &v[0], pos);
    SET_RETURN(val);
    return;
  }

  mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
}


//================================================================
/*! (method) unshift
*/
static void c_array_unshift(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_array_unshift(&v[0], &v[1]);
  mrbc_set_tt( &v[1], MRBC_TT_EMPTY );
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
    mrbc_value val = mrbc_array_shift(v);
    SET_RETURN(val);
    return;
  }

  /*
    in case of pop(n) -> Array
  */
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_INTEGER ) {
    mrbc_value val = mrbc_array_divide(vm, &v[0], v[1].i);

    // swap v[0] and val
    mrbc_array tmp = *v[0].array;
    v[0].array->data_size = val.array->data_size;
    v[0].array->n_stored = val.array->n_stored;
    v[0].array->data = val.array->data;

    val.array->data_size = tmp.data_size;
    val.array->n_stored = tmp.n_stored;
    val.array->data = tmp.data;

    SET_RETURN(val);
    return;
  }

  mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
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


//================================================================
/*! (method) uniq
*/
static void c_array_uniq(struct VM *vm, mrbc_value v[], int argc)
{
  // subset of Array#uniq

  if( mrbc_c_block_given(vm, v, argc) ) {
    mrbc_raise(vm, MRBC_CLASS(NotImplementedError), "Block are not supported");
    return;
  }

  mrbc_value ret = mrbc_array_uniq( vm, &v[0] );
  SET_RETURN( ret );
}


//================================================================
/*! (method) uniq!
*/
static void c_array_uniq_self(struct VM *vm, mrbc_value v[], int argc)
{
  // subset of Array#uniq!

  if( mrbc_c_block_given(vm, v, argc) ) {
    mrbc_raise(vm, MRBC_CLASS(NotImplementedError), "Block are not supported");
    return;
  }

  int n = mrbc_array_uniq_self( &v[0] );
  if( n == 0 ) SET_NIL_RETURN();
}


#if MRBC_USE_STRING
//================================================================
/*! (method) inspect, to_s
*/
static void c_array_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    v[0] = mrbc_string_new_cstr(vm, mrbc_symid_to_str( v[0].cls->sym_id ));
    return;
  }

  mrbc_value ret = mrbc_string_new_cstr(vm, "[");
  if( !ret.string ) goto RETURN_NIL;		// ENOMEM

  for( int i = 0; i < mrbc_array_size(v); i++ ) {
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
    if( mrbc_type(src->array->data[i]) == MRBC_TT_ARRAY ) {
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
  FILE("_autogen_class_array.h")

  METHOD( "new",	c_array_new )
  METHOD( "+",		c_array_add )
  METHOD( "-",		c_array_difference )
  METHOD( "[]",		c_array_get )
  METHOD( "at",		c_array_get )
  METHOD( "[]=",	c_array_set )
  METHOD( "<<",		c_array_push )
  METHOD( "clear",	c_array_clear )
  METHOD( "difference", c_array_difference )
  METHOD( "delete_at",	c_array_delete_at )
  METHOD( "empty?",	c_array_empty )
  METHOD( "size",	c_array_size )
  METHOD( "length",	c_array_size )
  METHOD( "count",	c_array_size )
  METHOD( "include?",	c_array_include )
  METHOD( "&",		c_array_and )
  METHOD( "|",		c_array_or )
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
  METHOD( "uniq",	c_array_uniq )
  METHOD( "uniq!",	c_array_uniq_self )
#if MRBC_USE_STRING
  METHOD( "inspect",	c_array_inspect )
  METHOD( "to_s",	c_array_inspect )
  METHOD( "join",	c_array_join )
#endif
*/
#include "_autogen_class_array.h"

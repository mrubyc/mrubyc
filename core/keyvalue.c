/*! @file
  @brief
  mruby/c Key(Symbol) - Value store.

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#include "vm_config.h"
#include <stdlib.h>
#include <string.h>

#include "value.h"
#include "alloc.h"
#include "keyvalue.h"

#if !defined(MRBC_KV_SIZE_INIT)
#define MRBC_KV_SIZE_INIT 2
#endif
#if !defined(MRBC_KV_SIZE_INCREMENT)
#define MRBC_KV_SIZE_INCREMENT 5
#endif


//================================================================
/*! binary search

  @param  kvh		pointer to key-value handle.
  @param  sym_id	symbol ID.
  @return		result. It's not necessarily found.
*/
static int binary_search(mrbc_kv_handle *kvh, mrbc_sym sym_id)
{
  int left = 0;
  int right = kvh->n_stored - 1;
  if( right < 0 ) return -1;

  while( left < right ) {
    int mid = (left + right) / 2;
    if( kvh->data[mid].sym_id < sym_id ) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }

  return left;
}


//================================================================
/*! constructor

  @param  vm	Pointer to VM.
  @param  size	Initial size of data.
  @return 	Key-Value handle.
*/
mrbc_kv_handle * mrbc_kv_new(struct VM *vm, int size)
{
  mrbc_kv_handle *kvh = mrbc_alloc(vm, sizeof(mrbc_kv_handle));
  if( !kvh ) return NULL;	// ENOMEM

  if( mrbc_kv_init_handle( vm, kvh, size ) != 0 ) {
    mrbc_raw_free( kvh );
    return NULL;
  }

  return kvh;
}


//================================================================
/*! initialize handle

  @param  vm	Pointer to VM.
  @param  kvh	Pointer to Key-Value handle.
  @param  size	Initial size of data.
  @return 	0 if no error.
*/
int mrbc_kv_init_handle(struct VM *vm, mrbc_kv_handle *kvh, int size)
{
  kvh->data_size = size;
  kvh->n_stored = 0;

  if( size == 0 ) {
    // save VM address temporary.
    kvh->vm = vm;

  } else {
    // Allocate data buffer.
    kvh->data = mrbc_alloc(vm, sizeof(mrbc_kv) * size);
    if( !kvh->data ) return -1;		// ENOMEM
  }

  return 0;
}


//================================================================
/*! destructor

  @param  kvh	pointer to key-value handle.
*/
void mrbc_kv_delete(mrbc_kv_handle *kvh)
{
  mrbc_kv_delete_data(kvh);
  mrbc_raw_free(kvh);
}


//================================================================
/*! delete all datas

  @param  kvh	pointer to key-value handle.
*/
void mrbc_kv_delete_data(mrbc_kv_handle *kvh)
{
  if( kvh->data_size == 0 ) return;

  mrbc_kv_clear(kvh);
  mrbc_raw_free(kvh->data);
}


//================================================================
/*! clear vm_id

  @param  kvh	pointer to key-value handle.
*/
void mrbc_kv_clear_vm_id(mrbc_kv_handle *kvh)
{
  mrbc_set_vm_id( kvh, 0 );
  if( kvh->data_size == 0 ) return;

  mrbc_kv *p1 = kvh->data;
  const mrbc_kv *p2 = p1 + kvh->n_stored;
  while( p1 < p2 ) {
    mrbc_clear_vm_id(&p1->value);
    p1++;
  }
}


//================================================================
/*! resize buffer

  @param  kvh	pointer to key-value handle.
  @param  size	size.
  @return	mrbc_error_code.
*/
int mrbc_kv_resize(mrbc_kv_handle *kvh, int size)
{
  mrbc_kv *data2 = mrbc_raw_realloc(kvh->data, sizeof(mrbc_kv) * size);
  if( !data2 ) return E_NOMEMORY_ERROR;		// ENOMEM

  kvh->data = data2;
  kvh->data_size = size;

  return 0;
}



//================================================================
/*! setter

  @param  kvh		pointer to key-value handle.
  @param  sym_id	symbol ID.
  @param  set_val	set value.
  @return		mrbc_error_code.
*/
int mrbc_kv_set(mrbc_kv_handle *kvh, mrbc_sym sym_id, mrbc_value *set_val)
{
  int idx = binary_search(kvh, sym_id);
  if( idx < 0 ) {
    idx = 0;
    goto INSERT_VALUE;
  }

  // replace value ?
  if( kvh->data[idx].sym_id == sym_id ) {
    mrbc_dec_ref_counter( &kvh->data[idx].value );
    kvh->data[idx].value = *set_val;
    return 0;
  }

  if( kvh->data[idx].sym_id < sym_id ) {
    idx++;
  }

 INSERT_VALUE:
  // need alloc?
  if( kvh->data_size == 0 ) {
    kvh->data = mrbc_alloc(kvh->vm, sizeof(mrbc_kv) * MRBC_KV_SIZE_INIT);
    if( kvh->data == NULL ) return E_NOMEMORY_ERROR;	// ENOMEM
    kvh->data_size = MRBC_KV_SIZE_INIT;

  // need resize?
  } else if( kvh->n_stored >= kvh->data_size ) {
    if( mrbc_kv_resize(kvh, kvh->data_size + MRBC_KV_SIZE_INCREMENT) != 0 ) {
      return E_NOMEMORY_ERROR;		// ENOMEM
    }

  // need move data?
  } else if( idx < kvh->n_stored ) {
    int size = sizeof(mrbc_kv) * (kvh->n_stored - idx);
    memmove( &kvh->data[idx+1], &kvh->data[idx], size );
  }

  kvh->data[idx].sym_id = sym_id;
  kvh->data[idx].value = *set_val;
  kvh->n_stored++;

  return 0;
}



//================================================================
/*! getter

  @param  kvh		pointer to key-value handle.
  @param  sym_id	symbol ID.
  @return		pointer to mrbc_value or NULL.
*/
mrbc_value * mrbc_kv_get(mrbc_kv_handle *kvh, mrbc_sym sym_id)
{
  int idx = binary_search(kvh, sym_id);
  if( idx < 0 ) return NULL;
  if( kvh->data[idx].sym_id != sym_id ) return NULL;

  return &kvh->data[idx].value;
}



//================================================================
/*! setter - only append tail

  @param  kvh		pointer to key-value handle.
  @param  sym_id	symbol ID.
  @param  set_val	set value.
  @return		mrbc_error_code.
*/
int mrbc_kv_append(mrbc_kv_handle *kvh, mrbc_sym sym_id, mrbc_value *set_val)
{
  // need alloc?
  if( kvh->data_size == 0 ) {
    kvh->data = mrbc_alloc(kvh->vm, sizeof(mrbc_kv) * MRBC_KV_SIZE_INIT);
    if( kvh->data == NULL ) return E_NOMEMORY_ERROR;	// ENOMEM
    kvh->data_size = MRBC_KV_SIZE_INIT;

  // need resize?
  } else if( kvh->n_stored >= kvh->data_size ) {
    if( mrbc_kv_resize(kvh, kvh->data_size + MRBC_KV_SIZE_INCREMENT) != 0 ) {
      return E_NOMEMORY_ERROR;		// ENOMEM
    }
  }

  kvh->data[kvh->n_stored].sym_id = sym_id;
  kvh->data[kvh->n_stored].value = *set_val;
  kvh->n_stored++;

  return 0;
}



static int compare_key( const void *kv1, const void *kv2 )
{
  return ((mrbc_kv *)kv1)->sym_id - ((mrbc_kv *)kv2)->sym_id;
}

//================================================================
/*! reorder

  @param  kvh		pointer to key-value handle.
  @return		mrbc_error_code.
*/
int mrbc_kv_reorder(mrbc_kv_handle *kvh)
{
  if( kvh->data_size == 0 ) return 0;

  qsort( kvh->data, kvh->n_stored, sizeof(mrbc_kv), compare_key );

  return 0;
}



//================================================================
/*! remove a data

  @param  kvh		pointer to key-value handle.
  @param  sym_id	symbol ID.
  @return		mrbc_error_code.
*/
int mrbc_kv_remove(mrbc_kv_handle *kvh, mrbc_sym sym_id)
{
  int idx = binary_search(kvh, sym_id);
  if( idx < 0 ) return 0;
  if( kvh->data[idx].sym_id != sym_id ) return 0;

  mrbc_dec_ref_counter( &kvh->data[idx].value );
  kvh->n_stored--;
  memmove( kvh->data + idx, kvh->data + idx + 1,
	   sizeof(mrbc_kv) * (kvh->n_stored - idx) );

  return 0;
}



//================================================================
/*! clear all

  @param  kvh		pointer to key-value handle.
*/
void mrbc_kv_clear(mrbc_kv_handle *kvh)
{
  mrbc_kv *p1 = kvh->data;
  const mrbc_kv *p2 = p1 + kvh->n_stored;
  while( p1 < p2 ) {
    mrbc_dec_ref_counter(&p1->value);
    p1++;
  }

  kvh->n_stored = 0;
}

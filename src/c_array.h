/*! @file
  @brief
  mruby/c Array class

  <pre>
  Copyright (C) 2015-2020 Kyushu Institute of Technology.
  Copyright (C) 2015-2020 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_ARRAY_H_
#define MRBC_SRC_C_ARRAY_H_

#include "alloc.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

//================================================================
/*!@brief
  Define Array handle.
*/
typedef struct RArray {
  MRBC_OBJECT_HEADER;

  uint16_t data_size;	//!< data buffer size.
  uint16_t n_stored;	//!< # of stored.
  mrbc_value *data;	//!< pointer to allocated memory.

} mrbc_array;


mrbc_value mrbc_array_new(struct VM *vm, int size);
void mrbc_array_delete(mrbc_value *ary);
void mrbc_array_clear_vm_id(mrbc_value *ary);
int mrbc_array_resize(mrbc_value *ary, int size);
int mrbc_array_set(mrbc_value *ary, int idx, mrbc_value *set_val);
mrbc_value mrbc_array_get(const mrbc_value *ary, int idx);
int mrbc_array_push(mrbc_value *ary, mrbc_value *set_val);
mrbc_value mrbc_array_pop(struct VM *vm, mrbc_value *ary, uint16_t len);
int mrbc_array_unshift(mrbc_value *ary, mrbc_value *set_val);
mrbc_value mrbc_array_shift(struct VM *vm, mrbc_value *ary, uint16_t len );
int mrbc_array_insert(mrbc_value *ary, int idx, mrbc_value *set_val);
mrbc_value mrbc_array_remove(mrbc_value *ary, int idx);
void mrbc_array_clear(mrbc_value *ary);
int mrbc_array_compare(const mrbc_value *v1, const mrbc_value *v2);
void mrbc_array_minmax(mrbc_value *ary, mrbc_value **pp_min_value, mrbc_value **pp_max_value);
mrbc_value mrbc_array_dup(struct VM *vm, const mrbc_value *ary);


//================================================================
/*! get size
*/
static inline int mrbc_array_size(const mrbc_value *ary)
{
  return ary->array->n_stored;
}


//================================================================
/*! delete handle (do not decrement reference counter)
*/
static inline void mrbc_array_delete_handle(mrbc_value *ary)
{
  mrbc_array *h = ary->array;

  mrbc_raw_free(h->data);
  mrbc_raw_free(h);
}


#ifdef __cplusplus
}
#endif
#endif

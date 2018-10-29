/*! @file
  @brief
  mruby/c Array class

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_ARRAY_H_
#define MRBC_SRC_C_ARRAY_H_

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
mrbc_value mrbc_array_get(mrbc_value *ary, int idx);
int mrbc_array_push(mrbc_value *ary, mrbc_value *set_val);
mrbc_value mrbc_array_pop(mrbc_value *ary);
int mrbc_array_unshift(mrbc_value *ary, mrbc_value *set_val);
mrbc_value mrbc_array_shift(mrbc_value *ary);
int mrbc_array_insert(mrbc_value *ary, int idx, mrbc_value *set_val);
mrbc_value mrbc_array_remove(mrbc_value *ary, int idx);
void mrbc_array_clear(mrbc_value *ary);
int mrbc_array_compare(const mrbc_value *v1, const mrbc_value *v2);
void mrbc_array_minmax(mrbc_value *ary, mrbc_value **pp_min_value, mrbc_value **pp_max_value);
void mrbc_init_class_array(struct VM *vm);


//================================================================
/*! get size
*/
static inline int mrbc_array_size(const mrbc_value *ary)
{
  return ary->array->n_stored;
}


#ifdef __cplusplus
}
#endif
#endif

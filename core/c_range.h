/*! @file
  @brief
  mruby/c Range object

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_RANGE_H_
#define MRBC_SRC_C_RANGE_H_

#include <stdint.h>
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

//================================================================
/*!@brief
  Define Range object (same the handles of other objects)
*/
typedef struct RRange {
  MRBC_OBJECT_HEADER;

  uint8_t flag_exclude;	// true: exclude the end object, otherwise include.
  mrbc_value first;
  mrbc_value last;

} mrbc_range;


mrbc_value mrbc_range_new(struct VM *vm, mrbc_value *first, mrbc_value *last, int flag_exclude);
void mrbc_range_delete(mrbc_value *v);
void mrbc_range_clear_vm_id(mrbc_value *v);
int mrbc_range_compare(const mrbc_value *v1, const mrbc_value *v2);
void mrbc_init_class_range(struct VM *vm);


//================================================================
/*! get first value
*/
static inline mrbc_value mrbc_range_first(const mrbc_value *v)
{
  return v->range->first;
}

//================================================================
/*! get last value
*/
static inline mrbc_value mrbc_range_last(const mrbc_value *v)
{
  return v->range->last;
}

//================================================================
/*! get exclude_end?
*/
static inline int mrbc_range_exclude_end(const mrbc_value *v)
{
  return v->range->flag_exclude;
}



#ifdef __cplusplus
}
#endif
#endif

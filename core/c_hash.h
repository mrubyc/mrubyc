/*! @file
  @brief
  mruby/c Hash class

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_HASH_H_
#define MRBC_SRC_C_HASH_H_

#include "value.h"
#include "c_array.h"

#ifdef __cplusplus
extern "C" {
#endif

//================================================================
/*!@brief
  Define Hash handle.
*/
typedef struct RHash {
  // (NOTE)
  //  Needs to be same members and order as RArray.
  MRBC_OBJECT_HEADER;

  uint16_t data_size;	//!< data buffer size.
  uint16_t n_stored;	//!< # of stored.
  mrbc_value *data;	//!< pointer to allocated memory.

  // TODO: and other member for search.

} mrbc_hash;


//================================================================
/*!@brief
  Define Hash iterator.
*/
typedef struct RHashIterator {
  mrbc_hash *target;
  mrbc_value *point;
  mrbc_value *p_end;
} mrbc_hash_iterator;


mrbc_value mrbc_hash_new(struct VM *vm, int size);
void mrbc_hash_delete(mrbc_value *hash);
mrbc_value *mrbc_hash_search(const mrbc_value *hash, const mrbc_value *key);
int mrbc_hash_set(mrbc_value *hash, mrbc_value *key, mrbc_value *val);
mrbc_value mrbc_hash_get(mrbc_value *hash, mrbc_value *key);
mrbc_value mrbc_hash_remove(mrbc_value *hash, mrbc_value *key);
void mrbc_hash_clear(mrbc_value *hash);
int mrbc_hash_compare(const mrbc_value *v1, const mrbc_value *v2);
mrbc_value mrbc_hash_dup(struct VM *vm, mrbc_value *src);
void mrbc_init_class_hash(struct VM *vm);



//================================================================
/*! get size
*/
static inline int mrbc_hash_size(const mrbc_value *hash) {
  return hash->hash->n_stored / 2;
}

//================================================================
/*! clear vm_id
*/
static inline void mrbc_hash_clear_vm_id(mrbc_value *hash) {
  mrbc_array_clear_vm_id(hash);
}

//================================================================
/*! resize buffer
*/
static inline int mrbc_hash_resize(mrbc_value *hash, int size)
{
  return mrbc_array_resize(hash, size * 2);
}


//================================================================
/*! iterator constructor
*/
static inline mrbc_hash_iterator mrbc_hash_iterator_new( mrbc_value *v )
{
  mrbc_hash_iterator ite;
  ite.target = v->hash;
  ite.point = v->hash->data;
  ite.p_end = ite.point + v->hash->n_stored;

  return ite;
}

//================================================================
/*! iterator has_next?
*/
static inline int mrbc_hash_i_has_next( mrbc_hash_iterator *ite )
{
  return ite->point < ite->p_end;
}

//================================================================
/*! iterator getter
*/
static inline mrbc_value *mrbc_hash_i_next( mrbc_hash_iterator *ite )
{
  mrbc_value *ret = ite->point;
  ite->point += 2;
  return ret;
}


#ifdef __cplusplus
}
#endif
#endif

/*! @file
  @brief
  mruby/c Hash class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_HASH_H_
#define MRBC_SRC_C_HASH_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include <stdint.h>
//@endcond

/***** Local headers ********************************************************/
#include "value.h"
#include "c_array.h"
#include "vm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  Hash object.

  @extends RBasic
*/
typedef struct RHash {
  // (NOTE)
  //  Needs to be same members and order as RArray.
  MRBC_OBJECT_HEADER;

  uint16_t data_size;	//!< data buffer size.
  uint16_t n_stored;	//!< num of stored.
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


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
mrbc_value mrbc_hash_new(mrbc_vm *vm, int size);
void mrbc_hash_delete(mrbc_value *hash);
mrbc_value *mrbc_hash_search(const mrbc_value *hash, const mrbc_value *key);
mrbc_value *mrbc_hash_search_by_id(const mrbc_value *hash, mrbc_sym sym_id);
int mrbc_hash_set(mrbc_value *hash, mrbc_value *key, mrbc_value *val);
mrbc_value mrbc_hash_get(const mrbc_value *hash, const mrbc_value *key);
mrbc_value *mrbc_hash_get_p(const mrbc_value *hash, const mrbc_value *key);
mrbc_value mrbc_hash_remove(mrbc_value *hash, const mrbc_value *key);
mrbc_value mrbc_hash_remove_by_id(mrbc_value *hash, mrbc_sym sym_id);
void mrbc_hash_clear(mrbc_value *hash);
int mrbc_hash_compare(const mrbc_value *v1, const mrbc_value *v2);
mrbc_value mrbc_hash_dup(mrbc_vm *vm, mrbc_value *src);
//@endcond

/***** Inline functions *****************************************************/
//================================================================
/*! get size
*/
static inline int mrbc_hash_size(const mrbc_value *hash) {
  return hash->hash->n_stored / 2;
}

#if defined(MRBC_ALLOC_VMID)
//================================================================
/*! clear vm_id
*/
static inline void mrbc_hash_clear_vm_id(mrbc_value *hash) {
  mrbc_array_clear_vm_id(hash);
}
#endif

//================================================================
/*! resize buffer
*/
static inline int mrbc_hash_resize(mrbc_value *hash, int size)
{
  return mrbc_array_resize(hash, size * 2);
}


//================================================================
/*! iterator constructor

<b>Code example</b>
@code
  mrbc_hash_iterator ite = mrbc_hash_iterator_new( hash );
  while( mrbc_hash_i_has_next( &ite ) ) {
    mrbc_value *kv = mrbc_hash_i_next( &ite );
    // using kv[0] as key, kv[1] as value
  }
@endcode
*/
static inline mrbc_hash_iterator mrbc_hash_iterator_new( const mrbc_value *v )
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

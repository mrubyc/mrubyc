/*! @file
  @brief
  mruby/c Key(Symbol) - Value store.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_KEYVALUE_H_
#define MRBC_SRC_C_KEYVALUE_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdint.h>
//@endcond

/***** Local headers ********************************************************/
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif


/***** Constant values ******************************************************/
/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  Key-Value data.
*/
typedef struct RKeyValue {
#if defined(MRBC_DEBUG)
  char obj_mark_[2];	//!< set "KV" for debug
#endif
  mrbc_sym sym_id;	//!< symbol ID as key.
  mrbc_value value;	//!< stored value.

} mrbc_kv;


//================================================================
/*!@brief
  Key-Value handle.
*/
typedef struct RKeyValueHandle {
  uint16_t data_size;	//!< data buffer size.
  uint16_t n_stored;	//!< num of stored.
  union {
    mrbc_kv *data;	//!< pointer to allocated memory.
    struct VM *vm;	//!< pointer to VM (if data_size == 0)
  };

} mrbc_kv_handle;

#define MRBC_KVH_INITIALIZER(vm) (mrbc_kv_handle){	\
    .data_size = 0,					\
    .n_stored = 0,					\
    .vm = vm,						\
}


//================================================================
/*!@brief
  Key-Value iterator.
*/
typedef struct RKeyValueIterator {
  const mrbc_kv_handle *target;
  uint16_t i;

} mrbc_kv_iterator;


/***** Macros ***************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
mrbc_kv_handle *mrbc_kv_new(struct VM *vm, int size);
int mrbc_kv_init_handle(struct VM *vm, mrbc_kv_handle *kvh, int size);
void mrbc_kv_delete(mrbc_kv_handle *kvh);
void mrbc_kv_delete_data(mrbc_kv_handle *kvh);
void mrbc_kv_clear_vm_id(mrbc_kv_handle *kvh);
int mrbc_kv_resize(mrbc_kv_handle *kvh, int size);
int mrbc_kv_set(mrbc_kv_handle *kvh, mrbc_sym sym_id, mrbc_value *set_val);
mrbc_value *mrbc_kv_get(mrbc_kv_handle *kvh, mrbc_sym sym_id);
int mrbc_kv_append(mrbc_kv_handle *kvh, mrbc_sym sym_id, mrbc_value *set_val);
int mrbc_kv_reorder(mrbc_kv_handle *kvh);
int mrbc_kv_remove(mrbc_kv_handle *kvh, mrbc_sym sym_id);
void mrbc_kv_clear(mrbc_kv_handle *kvh);
void mrbc_kv_dup(const mrbc_kv_handle *src, mrbc_kv_handle *dst);
//@endcond


/***** Inline functions *****************************************************/
//================================================================
/*! get size
*/
static inline int mrbc_kv_size(const mrbc_kv_handle *kvh)
{
  return kvh->n_stored;
}

//================================================================
/*! iterator: constructor

<b>Code example</b>
@code
  mrbc_kv_iterator ite = mrbc_kv_iterator_new( kv_handle );
  while( mrbc_kv_i_has_next( &ite ) ) {
    const mrbc_kv *kv = mrbc_kv_i_next( &ite );
    // using kv->sym_id and kv->value.
  }
@endcode
*/
static inline mrbc_kv_iterator mrbc_kv_iterator_new( const mrbc_kv_handle *h )
{
  mrbc_kv_iterator ite;

  ite.target = h;
  ite.i = 0;

  return ite;
}

//================================================================
/*! iterator: is_first?
*/
static inline int mrbc_kv_i_is_first( const mrbc_kv_iterator *ite )
{
  return ite->i == 0;
}

//================================================================
/*! iterator: has_next?
*/
static inline int mrbc_kv_i_has_next( const mrbc_kv_iterator *ite )
{
  return ite->i < ite->target->n_stored;
}

//================================================================
/*! iterator: getter
*/
static inline mrbc_kv *mrbc_kv_i_get( mrbc_kv_iterator *ite )
{
  return &ite->target->data[ ite->i ];
}

//================================================================
/*! iterator: get and next
*/
static inline mrbc_kv *mrbc_kv_i_next( mrbc_kv_iterator *ite )
{
  return &ite->target->data[ ite->i++ ];
}


#ifdef __cplusplus
}
#endif
#endif

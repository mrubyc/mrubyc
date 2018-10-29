/*! @file
  @brief
  mruby/c String object

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_STRING_H_
#define MRBC_SRC_C_STRING_H_

#include <stdint.h>
#include <string.h>
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

//================================================================
/*!@brief
  Define String handle.
*/
typedef struct RString {
  MRBC_OBJECT_HEADER;

  uint16_t size;	//!< string length.
  uint8_t *data;	//!< pointer to allocated buffer.

} mrbc_string;


mrbc_value mrbc_string_new(struct VM *vm, const void *src, int len);
mrbc_value mrbc_string_new_cstr(struct VM *vm, const char *src);
mrbc_value mrbc_string_new_alloc(struct VM *vm, void *buf, int len);
void mrbc_string_delete(mrbc_value *str);
void mrbc_string_clear_vm_id(mrbc_value *str);
mrbc_value mrbc_string_dup(struct VM *vm, mrbc_value *s1);
mrbc_value mrbc_string_add(struct VM *vm, const mrbc_value *s1, const mrbc_value *s2);
int mrbc_string_append(mrbc_value *s1, const mrbc_value *s2);
int mrbc_string_append_cstr(mrbc_value *s1, const char *s2);
int mrbc_string_index(const mrbc_value *src, const mrbc_value *pattern, int offset);
int mrbc_string_strip(mrbc_value *src, int mode);
int mrbc_string_chomp(mrbc_value *src);
void mrbc_init_class_string(struct VM *vm);


//================================================================
/*! compare
*/
static inline int mrbc_string_compare(const mrbc_value *v1, const mrbc_value *v2)
{
  int len = (v1->string->size < v2->string->size) ?
    v1->string->size : v2->string->size;

  int res = memcmp(v1->string->data, v2->string->data, len);
  if( res != 0 ) return res;

  return v1->string->size - v2->string->size;
}

//================================================================
/*! get size
*/
static inline int mrbc_string_size(const mrbc_value *str)
{
  return str->string->size;
}

//================================================================
/*! get c-language string (char *)
*/
static inline char * mrbc_string_cstr(const mrbc_value *v)
{
  return (char*)v->string->data;
}


#ifdef __cplusplus
}
#endif
#endif

/*! @file
  @brief
  mruby/c Symbol class

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_SYMBOL_H_
#define MRBC_SRC_SYMBOL_H_

#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif


mrbc_value mrbc_symbol_new(struct VM *vm, const char *str);
uint16_t calc_hash(const char *str);
mrbc_sym str_to_symid(const char *str);
const char *symid_to_str(mrbc_sym sym_id);
void mrbc_init_class_symbol(struct VM *vm);

#if defined(MRBC_DEBUG)
void mrbc_symbol_statistics( int *total_used );
#endif


//================================================================
/*! get c-language string (char *)
*/
static inline const char * mrbc_symbol_cstr(const mrbc_value *v)
{
  return symid_to_str(v->i);
}


#ifdef __cplusplus
}
#endif
#endif

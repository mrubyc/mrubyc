/*! @file
  @brief
  mruby/c Integer and Float class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#ifndef MRBC_SRC_C_NUMERIC_H_
#define MRBC_SRC_C_NUMERIC_H_

#include "vm_config.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MRBC_USE_STRING_UTF8
int mrbc_utf8_encode(mrbc_int_t codepoint, char *buf);
#endif

#ifdef __cplusplus
}
#endif
#endif

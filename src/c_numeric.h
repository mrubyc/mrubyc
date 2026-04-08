/*! @file
  @brief
  mruby/c Integer and Float class

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_NUMERIC_H_
#define MRBC_SRC_C_NUMERIC_H_

//@cond
#include <stdint.h>
//@endcond

#ifdef __cplusplus
extern "C" {
#endif

int mrbc_utf8_encode(int32_t codepoint, char *buf);

#ifdef __cplusplus
}
#endif
#endif

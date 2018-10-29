/*! @file
  @brief
  Declare static data.

  <pre>
  Copyright (C) 2015-2016 Kyushu Institute of Technology.
  Copyright (C) 2015-2016 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

#ifndef MRBC_SRC_STATIC_H_
#define MRBC_SRC_STATIC_H_

#include "class.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Class Tree */
extern mrbc_class *mrbc_class_object;

extern mrbc_class *mrbc_class_proc;
extern mrbc_class *mrbc_class_false;
extern mrbc_class *mrbc_class_true;
extern mrbc_class *mrbc_class_nil;
extern mrbc_class *mrbc_class_array;
extern mrbc_class *mrbc_class_fixnum;
extern mrbc_class *mrbc_class_float;
extern mrbc_class *mrbc_class_math;
extern mrbc_class *mrbc_class_string;
extern mrbc_class *mrbc_class_symbol;
extern mrbc_class *mrbc_class_range;
extern mrbc_class *mrbc_class_hash;


void init_static(void);


#ifdef __cplusplus
}
#endif
#endif

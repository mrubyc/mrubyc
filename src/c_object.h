/*! @file
  @brief
  Object, Nil, True and False class.

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_OBJECT_H_
#define MRBC_SRC_OBJECT_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
/***** Local headers ********************************************************/
#include "value.h"
#include "vm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
void mrbc_instance_call_initialize(mrbc_vm *vm, mrbc_value v[], int argc);
void mrbc_object_inspect(mrbc_vm *vm, mrbc_value v[], int argc);
//@endcond


/***** Inline functions *****************************************************/


#ifdef __cplusplus
}
#endif
#endif

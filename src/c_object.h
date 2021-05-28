/*! @file
  @brief
  Object, Proc, Nil, True and False class.

  <pre>
  Copyright (C) 2015-2021 Kyushu Institute of Technology.
  Copyright (C) 2015-2021 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_OBJECT_H_
#define MRBC_SRC_OBJECT_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
/***** Local headers ********************************************************/
#include "value.h"

/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void c_proc_call(struct VM *vm, mrbc_value v[], int argc);

/***** Inline functions *****************************************************/


#ifdef __cplusplus
}
#endif
#endif

/*! @file
  @brief
  mruby bytecode loader.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_LOAD_H_
#define MRBC_SRC_LOAD_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include <stdint.h>
//@endcond

/***** Local headers ********************************************************/
#include "value.h"
#include "vm.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
// pre define of some struct
struct IREP;

/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
int mrbc_load_mrb(mrbc_vm *vm, const void *bytecode);
int mrbc_load_irep(mrbc_vm *vm, const void *bytecode);
void mrbc_irep_free(mrbc_irep *irep);
mrbc_value mrbc_irep_pool_value(mrbc_vm *vm, int n);
//@endcond


/***** Inline functions *****************************************************/

#ifdef __cplusplus
}
#endif
#endif

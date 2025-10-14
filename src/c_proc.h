/*! @file
  @brief
  mruby/c Proc class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_C_PROC_H_
#define MRBC_SRC_C_PROC_H_

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

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  Proc object.

  @extends RBasic
*/
typedef struct RProc {
  MRBC_OBJECT_HEADER;

  uint8_t          block_or_method;	//!< 'B' or 'M' char code
  struct CALLINFO *callinfo;		//!< callinfo when proc was created.
  struct CALLINFO *callinfo_self;	//!< callinfo of self object. Valid when 'B'.
  struct IREP     *irep;		//!< Target IREP.
  mrbc_value       self;		//!< Copy of self object. Valid when 'B'.
  mrbc_value       ret_val;		//!< Return value of this block.

} mrbc_proc;
//@cond
typedef struct RProc mrb_proc;
//@endcond

/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
mrbc_value mrbc_proc_new(struct VM *vm, void *irep, uint8_t b_or_m);
void mrbc_proc_delete(mrbc_value *val);
void mrbc_proc_clear_vm_id(mrbc_value *v);
//@endcond


/***** Inline functions *****************************************************/


#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_SRC_C_PROC_H_

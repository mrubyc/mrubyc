/*! @file
  @brief
  Task::Queue for mruby/c

  <pre>
  Copyright (C) 2026-      HASUMI Hitoshi

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_TASK_QUEUE_H_
#define MRBC_SRC_TASK_QUEUE_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
/***** Local headers ********************************************************/
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
void mrbc_init_task_queue(void);
//@endcond

/***** Inline functions *****************************************************/

#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_SRC_TASK_QUEUE_H_

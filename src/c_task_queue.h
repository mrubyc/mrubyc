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
//================================================================
/*!@brief
  Result of mrbc_task_queue_push().
*/
typedef enum {
  MRBC_TASK_QUEUE_PUSH_OK = 0,	//!< pushed. no task was waiting.
  MRBC_TASK_QUEUE_PUSH_OK_WOKE,	//!< pushed and a waiting task was woken.
  MRBC_TASK_QUEUE_PUSH_CLOSED,	//!< the queue is closed.
  MRBC_TASK_QUEUE_PUSH_INVALID,	//!< not a Task::Queue instance.

} mrbc_task_queue_push_result;


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
void mrbc_init_task_queue(void);
mrbc_task_queue_push_result mrbc_task_queue_push(mrbc_value *queue, mrbc_value *value);
//@endcond

/***** Inline functions *****************************************************/

#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_SRC_TASK_QUEUE_H_

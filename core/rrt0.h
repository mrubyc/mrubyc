/*! @file
  @brief
  Realtime multitask monitor for mruby/c

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

#ifndef MRBC_SRC_RRT0_H_
#define MRBC_SRC_RRT0_H_

#ifdef __cplusplus
extern "C" {
#endif

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdint.h>

/***** Local headers ********************************************************/
#include "vm.h"

/***** Constant values ******************************************************/

//================================================
/*!@brief
  Task state
*/
enum MrbcTaskState {
  TASKSTATE_DORMANT   = 0x00,
  TASKSTATE_READY     = 0x01,
  TASKSTATE_RUNNING   = 0x03,
  TASKSTATE_WAITING   = 0x04,
  TASKSTATE_SUSPENDED = 0x08,
};

enum MrbcTaskReason {
  TASKREASON_SLEEP = 0x00,
  TASKREASON_MUTEX = 0x01,
};


/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/

struct RMutex;

//================================================
/*!@brief
  Task control block
*/
typedef struct RTcb {
  struct RTcb *next;
  uint8_t priority;
  uint8_t priority_preemption;
  uint8_t timeslice;
  uint8_t state;	//!< enum MrbcTaskState
  uint8_t reason;	//!< SLEEP, MUTEX

  union {
    uint32_t wakeup_tick;
    struct RMutex *mutex;
  };
  struct VM vm;
} mrbc_tcb;



//================================================
/*!@brief
  Mutex
*/
typedef struct RMutex {
  volatile int lock;
  struct RTcb *tcb;
} mrbc_mutex;

#define MRBC_MUTEX_INITIALIZER { 0 }


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
void mrbc_tick(void);
void mrbc_init(uint8_t *ptr, unsigned int size);
void mrbc_init_tcb(mrbc_tcb *tcb);
mrbc_tcb *mrbc_create_task(const uint8_t *vm_code, mrbc_tcb *tcb);
int mrbc_start_task(mrbc_tcb *tcb);
int mrbc_run(void);
void mrbc_sleep_ms(mrbc_tcb *tcb, uint32_t ms);
void mrbc_relinquish(mrbc_tcb *tcb);
void mrbc_change_priority(mrbc_tcb *tcb, int priority);
void mrbc_suspend_task(mrbc_tcb *tcb);
void mrbc_resume_task(mrbc_tcb *tcb);
mrbc_mutex *mrbc_mutex_init(mrbc_mutex *mutex);
int mrbc_mutex_lock(mrbc_mutex *mutex, mrbc_tcb *tcb);
int mrbc_mutex_unlock(mrbc_mutex *mutex, mrbc_tcb *tcb);
int mrbc_mutex_trylock(mrbc_mutex *mutex, mrbc_tcb *tcb);


/***** Inline functions *****************************************************/


#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_SRC_RRT0_H_

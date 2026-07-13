/*! @file
  @brief
  Realtime multitask monitor for mruby/c

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

/***** Macros ***************************************************************/
#ifndef MRBC_SCHEDULER_EXIT
#define MRBC_SCHEDULER_EXIT 0
#endif

#define MRBC_MUTEX_TRACE(...) ((void)0)

#if !defined(MRBC_TIMESLICE_TICK_COUNT) || (MRBC_TIMESLICE_TICK_COUNT <= 0)
#error "MRBC_TIMESLICE_TICK_COUNT must be a natural number."
#endif
#define DEPRECATED(msg) do { \
    static const char msg1[] = "Warning: "; \
    static const char msg2[] = " method will be removed in future version.\n"; \
    mrbc_hal_write(2, msg1, sizeof(msg1)-1); \
    mrbc_hal_write(2, msg, strlen(msg)); \
    mrbc_hal_write(2, msg2, sizeof(msg2)-1); \
  } while(0)

/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
#define NUM_TCB_QUEUE 4
static mrbc_tcb *tcb_queue_[NUM_TCB_QUEUE];
#define q_dormant_   (tcb_queue_[0])
#define q_ready_     (tcb_queue_[1])
#define q_waiting_   (tcb_queue_[2])
#define q_suspended_ (tcb_queue_[3])
static volatile uint32_t tick_;
static volatile uint32_t wakeup_tick_ = ((uint32_t)1 << 16); // no significant meaning.


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Functions ************************************************************/
//================================================================
/*! Insert task(TCB) to task queue

  @param  p_tcb	Pointer to target TCB

  Put the task (TCB) into a queue by each state.
  TCB must be free. (must not be in another queue)
  The queue is sorted in priority_preemption order.
  If the same priority_preemption value is in the TCB and queue,
  it will be inserted at the end of the same value in queue.
*/
void mrbc_task_q_insert(mrbc_tcb *p_tcb)
{
  // select target queue pointer.
  //                    state value = 0  1  2  3  4  5  6  7  8
  //                             /2   0, 0, 1, 1, 2, 2, 3, 3, 4
  static const uint8_t conv_tbl[] = { 0,    1,    2,    0,    3 };
  mrbc_tcb **pp_q = &tcb_queue_[ conv_tbl[ p_tcb->state / 2 ]];

  // in case of insert on top.
  if((*pp_q == NULL) ||
     (p_tcb->priority_preemption < (*pp_q)->priority_preemption)) {
    p_tcb->next = *pp_q;
    *pp_q       = p_tcb;
    return;
  }

  // find insert point in sorted linked list.
  mrbc_tcb *p = *pp_q;
  while( p->next != NULL ) {
    if( p_tcb->priority_preemption < p->next->priority_preemption ) break;
    p = p->next;
  }

  // insert tcb to queue.
  p_tcb->next = p->next;
  p->next     = p_tcb;
}


//================================================================
/*! Delete task(TCB) from task queue

  @param  p_tcb	Pointer to target TCB
*/
void mrbc_task_q_delete(mrbc_tcb *p_tcb)
{
  // select target queue pointer. (same as mrbc_task_q_insert)
  static const uint8_t conv_tbl[] = { 0,    1,    2,    0,    3 };
  mrbc_tcb **pp_q = &tcb_queue_[ conv_tbl[ p_tcb->state / 2 ]];

  if( *pp_q == p_tcb ) {
    *pp_q       = p_tcb->next;
    p_tcb->next = NULL;
    return;
  }

  mrbc_tcb *p = *pp_q;
  while( p ) {
    if( p->next == p_tcb ) {
      p->next     = p_tcb->next;
      p_tcb->next = NULL;
      return;
    }

    p = p->next;
  }

  assert(!"Not found target task in queue.");
}


//================================================================
/*! get the head of the WAITING task queue.

  Provided so that out-of-file features (e.g. Task::Queue) can scan the
  tasks that are currently in the WAITING state.

  @return	Pointer to the first TCB in the waiting queue, or NULL.
*/
mrbc_tcb *mrbc_task_q_waiting_head(void)
{
  return q_waiting_;
}


//================================================================
/*! preempt running task
*/
inline static void preempt_running_task(void)
{
  for( mrbc_tcb *t = q_ready_; t != NULL; t = t->next ) {
    if( t->state == TASKSTATE_RUNNING ) t->vm.flag_preemption = 1;
  }
}


//================================================================
/*! Get the timed wakeup tick of a waiting task, if it has one.

  A SLEEP task always has a deadline; a QUEUE task has one only when popped
  with a timeout (queue.wakeup_tick != MRBC_WAIT_FOREVER). Other wait reasons
  have no timed wakeup.

  @param  t	task control block.
  @param  out	receives the wakeup tick when the function returns non-zero.
  @return	non-zero if the task has a timed wakeup.
*/
static int task_timed_wakeup(const mrbc_tcb *t, uint32_t *out)
{
  if( t->reason == TASKREASON_SLEEP ) {
    *out = t->wakeup_tick;
    return 1;
  }
  if( t->reason == TASKREASON_QUEUE && t->queue.wakeup_tick != MRBC_WAIT_FOREVER ) {
    *out = t->queue.wakeup_tick;
    return 1;
  }
  return 0;
}


//================================================================
/*! Tick timer interrupt handler.

*/
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE
#endif
void mrbc_tick(void)
{
  tick_++;

  // Decrease the time slice value for running tasks.
  mrbc_tcb *tcb = q_ready_;
  if( (tcb != NULL) && (tcb->timeslice != 0) ) {
    tcb->timeslice--;
    if( tcb->timeslice == 0 ) tcb->vm.flag_preemption = 1;
  }

  // Check the wakeup tick.
  if( (int32_t)(wakeup_tick_ - tick_) < 0 ) {
    int flag_preemption = 0;
    wakeup_tick_ = tick_ + ((uint32_t)1 << 16);

    // Find a wake up task in waiting task queue. Both sleeping tasks and
    // tasks blocked on a queue with a timeout carry a deadline.
    tcb = q_waiting_;
    while( tcb != NULL ) {
      mrbc_tcb *t = tcb;
      tcb = tcb->next;
      uint32_t wake;
      if( !task_timed_wakeup(t, &wake) ) continue;

      if( (int32_t)(wake - tick_) < 0 ) {
        mrbc_task_q_delete(t);
        t->state  = TASKSTATE_READY;
        t->reason = 0;
        mrbc_task_q_insert(t);
        flag_preemption = 1;
      } else if( (int32_t)(wake - wakeup_tick_) < 0 ) {
        wakeup_tick_ = wake;
      }
    }

    if( flag_preemption ) preempt_running_task();
  }
}


//================================================================
/*! create (allocate) TCB.

  @param  regs_size	num of allocated registers.
  @param  task_state	task initial state.
  @param  priority	task priority.
  @return pointer to TCB.

<b>Code example</b>
@code
  //  If you want specify default value, see below.
  //    regs_size:  MAX_REGS_SIZE (in vm_config.h)
  //    task_state: MRBC_TASK_DEFAULT_STATE
  //    priority:   MRBC_TASK_DEFAULT_PRIORITY
  mrbc_tcb *tcb;
  tcb = mrbc_tcb_new( MAX_REGS_SIZE, MRBC_TASK_DEFAULT_STATE, MRBC_TASK_DEFAULT_PRIORITY );
  mrbc_create_task( byte_code, tcb );
@endcode
*/
mrbc_tcb * mrbc_tcb_new( int regs_size, enum MrbcTaskState task_state, int priority )
{
  mrbc_tcb *tcb;
  unsigned int size = sizeof(mrbc_tcb) + sizeof(mrbc_value) * regs_size;

  tcb = mrbc_raw_alloc(size);
  memset(tcb, 0, size);
#if defined(MRBC_DEBUG)
  memcpy( tcb->obj_mark_, "TCB", 4 );
#endif
  tcb->priority = priority;
  tcb->state = task_state;
  tcb->vm.regs_size = regs_size;

  return tcb;
}


//================================================================
/*! Create a task specifying bytecode to be executed.

  @param  byte_code	pointer to VM byte code.
  @param  tcb		Task control block with parameter, or NULL.
  @return Pointer to mrbc_tcb or NULL.
*/
mrbc_tcb * mrbc_create_task(const void *byte_code, mrbc_tcb *tcb)
{
  if( !tcb ) tcb = mrbc_tcb_new( MAX_REGS_SIZE, MRBC_TASK_DEFAULT_STATE, MRBC_TASK_DEFAULT_PRIORITY );

  tcb->priority_preemption = tcb->priority;

  // assign VM ID
  if( mrbc_vm_open( &tcb->vm ) == NULL ) {
    mrbc_printf("Error: Can't assign VM-ID.\n");
    return NULL;
  }

  if( mrbc_load_mrb(&tcb->vm, byte_code) != 0 ) {
    mrbc_print_vm_exception( &tcb->vm );
    mrbc_vm_close( &tcb->vm );
    return NULL;
  }
  mrbc_vm_begin( &tcb->vm );

  mrbc_hal_disable_irq();
  mrbc_task_q_insert(tcb);
  if( tcb->state & TASKSTATE_READY ) preempt_running_task();
  mrbc_hal_enable_irq();

  return tcb;
}


//================================================================
/*! Delete a task.

  @param  tcb	target task.
  @return	0 on success, or a negative value on error.
*/
int mrbc_delete_task(mrbc_tcb *tcb)
{
  if( tcb->state != TASKSTATE_DORMANT )  return -1;

  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);
  mrbc_hal_enable_irq();

  mrbc_vm_close( &tcb->vm );

  return 0;
}


//================================================================
/*! set the task name.

  @param  tcb	target task.
  @param  name	task name
*/
void mrbc_set_task_name(mrbc_tcb *tcb, const char *name)
{
  /* (note)
   this is `strncpy( tcb->name, name, MRBC_TASK_NAME_LEN );`
   for to avoid link error when compiling for PIC32 with XC32 v4.21
  */
  for( int i = 0; i < MRBC_TASK_NAME_LEN; i++ ) {
    if( (tcb->name[i] = *name++) == 0 ) break;
  }
}


//================================================================
/*! find task by name

  @param  name		task name
  @return pointer to mrbc_tcb or NULL
*/
mrbc_tcb * mrbc_find_task(const char *name)
{
  mrbc_tcb *tcb = NULL;
  mrbc_hal_disable_irq();

  for( int i = 0; i < NUM_TCB_QUEUE; i++ ) {
    for( tcb = tcb_queue_[i]; tcb != NULL; tcb = tcb->next ) {
      if( strcmp( tcb->name, name ) == 0 ) goto RETURN_TCB;
    }
  }

 RETURN_TCB:
  mrbc_hal_enable_irq();
  return tcb;
}


//================================================================
/*! Start execution of dormant task.

  @param  tcb	target task.
  @return	0 on success, or a negative value on error.
*/
int mrbc_start_task(mrbc_tcb *tcb)
{
  if( tcb->state != TASKSTATE_DORMANT ) return -1;

  mrbc_hal_disable_irq();

  preempt_running_task();

  mrbc_task_q_delete(tcb);
  tcb->state = TASKSTATE_READY;
  tcb->reason = 0;
  tcb->priority_preemption = tcb->priority;
  mrbc_task_q_insert(tcb);

  mrbc_hal_enable_irq();

  return 0;
}


//----------------------------------------------------------------
static void terminate_task( mrbc_tcb *tcb )
{
  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);
  tcb->state = TASKSTATE_DORMANT;
  mrbc_task_q_insert(tcb);
  mrbc_hal_enable_irq();

  if( ! tcb->vm.flag_permanence ) mrbc_vm_end( &tcb->vm );

  // find task that called join.
  mrbc_hal_disable_irq();
  for( mrbc_tcb *t = q_waiting_; t != NULL; t = t->next ) {
    if( t->reason == TASKREASON_JOIN && t->tcb_join == tcb ) {
      mrbc_task_q_delete(t);
      t->state = TASKSTATE_READY;
      t->reason = 0;
      mrbc_task_q_insert(t);
    }
  }
  mrbc_hal_enable_irq();

  for( mrbc_tcb *t = q_suspended_; t != NULL; t = t->next ) {
    if( t->reason == TASKREASON_JOIN && t->tcb_join == tcb ) {
      t->reason = 0;
    }
  }
}


//================================================================
/*! execute

*/
int mrbc_run(void)
{
  int ret = 0;
  (void)ret;	// avoid warning.

  while( 1 ) {
    mrbc_tcb *tcb = q_ready_;
    if( tcb == NULL ) {		// no task to run.
#if MRBC_SCHEDULER_EXIT
      mrbc_hal_disable_irq();
      int flag_exit = !q_ready_ && !q_waiting_ && !q_suspended_;
      mrbc_hal_enable_irq();
      if( flag_exit ) return ret;
#endif
      mrbc_hal_idle_cpu();
      continue;
    }

    /*
      run the task.
    */
    tcb->state = TASKSTATE_RUNNING;   // to execute.
    tcb->timeslice = MRBC_TIMESLICE_TICK_COUNT;

#if !defined(MRBC_NO_TIMER)
    // Using hardware timer.
    int ret_vm_run = mrbc_vm_run(&tcb->vm);
    tcb->vm.flag_preemption = 0;
#else
    // Emulate time slice preemption.
    int ret_vm_run = 0;
    tcb->vm.flag_preemption = 1;
    while( tcb->timeslice != 0 ) {
      ret_vm_run = mrbc_vm_run( &tcb->vm );
      tcb->timeslice--;
      if( ret_vm_run != 0 ) break;
      if( tcb->state != TASKSTATE_RUNNING ) break;
    }
    mrbc_tick();
#endif

    /*
      did the task done?
    */
    if( ret_vm_run != 0 ) {
      terminate_task( tcb );
      if( ret_vm_run != 1 ) ret = ret_vm_run;   // for debug info.
      continue;
    }

    /*
      Switch task.
    */
    if( tcb->state == TASKSTATE_RUNNING ) {
      tcb->state = TASKSTATE_READY;

      mrbc_hal_disable_irq();
      mrbc_task_q_delete(tcb);       // insert task on queue last.
      mrbc_task_q_insert(tcb);
      mrbc_hal_enable_irq();
    }

  } // loop infinite.
}


//================================================================
/*! Alternative to mrbc_run for Wasm build

*/
#if defined(__EMSCRIPTEN__)
EMSCRIPTEN_KEEPALIVE
int
mrbc_run_step(void)
{
  // Take the task that can be executed
  mrbc_tcb *tcb = q_ready_;
  if (tcb == NULL) {
    // Even if there is no task to run, return 0
    // so to wait for callbacks like event listener
    return 0;
  }

  tcb->state = TASKSTATE_RUNNING;
  tcb->timeslice = MRBC_TIMESLICE_TICK_COUNT;

  int ret_vm_run = mrbc_vm_run(&tcb->vm);
  tcb->vm.flag_preemption = 0;

  /*
    did the task done?
  */
  if (ret_vm_run != 0) {
    terminate_task( tcb );
    return ret_vm_run;
  }

  // Switch task.
  if (tcb->state == TASKSTATE_RUNNING) {
    tcb->state = TASKSTATE_READY;
    mrbc_hal_disable_irq();
    mrbc_task_q_delete(tcb);
    mrbc_task_q_insert(tcb);
    mrbc_hal_enable_irq();
  }

  return 0;
}
#endif


//================================================================
/*! sleep for a specified number of milliseconds.

  @param  tcb	target task.
  @param  ms	sleep milliseconds.
*/
void mrbc_sleep_ms(mrbc_tcb *tcb, uint32_t ms)
{
  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);
  tcb->state       = TASKSTATE_WAITING;
  tcb->reason      = TASKREASON_SLEEP;
  tcb->wakeup_tick = tick_ + (ms / MRBC_TICK_UNIT) + !!(ms % MRBC_TICK_UNIT);

  if( (int32_t)(tcb->wakeup_tick - wakeup_tick_) < 0 ) {
    wakeup_tick_ = tcb->wakeup_tick;
  }

  mrbc_task_q_insert(tcb);
  mrbc_hal_enable_irq();

  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! Convert a millisecond timeout into an absolute wakeup tick (deadline).

  @param  ms		timeout in milliseconds (must be >= 0).
  @param  p_overflow	set to non-zero when the deadline is too far ahead to
			be expressed in the 31-bit signed tick window.
  @return		absolute deadline tick, normalized so that it never
			equals the MRBC_WAIT_FOREVER sentinel.
*/
uint32_t mrbc_deadline_after_ms(mrbc_int_t ms, int *p_overflow)
{
  int64_t ticks = (int64_t)(ms / MRBC_TICK_UNIT) + !!(ms % MRBC_TICK_UNIT);
  if( ticks > INT32_MAX ) {
    *p_overflow = 1;
    return 0;
  }
  *p_overflow = 0;

  uint32_t deadline = tick_ + (uint32_t)ticks;
  // Never hand back the "no timeout" sentinel for a real deadline; pulling it
  // back one tick costs at most one tick in the 1-in-2^32 collision case.
  return deadline == MRBC_WAIT_FOREVER ? deadline - 1 : deadline;
}


//================================================================
/*! Test whether an absolute deadline tick has been reached.

  @param  deadline	absolute deadline tick from mrbc_deadline_after_ms().
  @return		non-zero if the current tick is at or past the deadline.
*/
int mrbc_deadline_reached(uint32_t deadline)
{
  return (int32_t)(deadline - tick_) <= 0;
}


//================================================================
/*! Lower the global next-wakeup tick to account for a new deadline.

  The caller must hold the IRQ lock; this only updates wakeup_tick_ when the
  given deadline is earlier than the currently scheduled one.

  @param  wakeup_tick	absolute deadline tick to register.
*/
void mrbc_register_wakeup(uint32_t wakeup_tick)
{
  if( (int32_t)(wakeup_tick - wakeup_tick_) < 0 ) {
    wakeup_tick_ = wakeup_tick;
  }
}


//================================================================
/*! wake up the task.

  @param  tcb		target task.
*/
void mrbc_wakeup_task(mrbc_tcb *tcb)
{
  switch( tcb->state ) {
  case TASKSTATE_SUSPENDED:
    mrbc_resume_task( tcb );    // for sleep without arguments.
    break;

  case TASKSTATE_WAITING:
    if( tcb->reason != TASKREASON_SLEEP ) break;

    mrbc_hal_disable_irq();
    mrbc_task_q_delete(tcb);
    tcb->state = TASKSTATE_READY;
    tcb->reason = 0;
    mrbc_task_q_insert(tcb);

    for( mrbc_tcb *t = q_waiting_; t != NULL; t = t->next ) {
      uint32_t wake;
      if( !task_timed_wakeup(t, &wake) ) continue;
      if( (int32_t)(wake - wakeup_tick_) < 0 ) {
        wakeup_tick_ = wake;
      }
    }
    mrbc_hal_enable_irq();
    break;

  default:
    break;
  }
}


//================================================================
/*! Relinquish control to other tasks.

  @param  tcb	target task.
*/
void mrbc_relinquish(mrbc_tcb *tcb)
{
  tcb->timeslice          = 0;
  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! change task priority.

  @param  tcb		target task.
  @param  priority	priority value. between 1 and 255.
*/
void mrbc_change_priority(mrbc_tcb *tcb, int priority)
{
  tcb->priority            = priority;
  tcb->priority_preemption = priority;

  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);       // reorder task queue according to priority.
  mrbc_task_q_insert(tcb);

  if( tcb->state & TASKSTATE_READY ) preempt_running_task();

  mrbc_hal_enable_irq();
}


//================================================================
/*! Suspend the task.

  @param  tcb		target task.
*/
void mrbc_suspend_task(mrbc_tcb *tcb)
{
  if( tcb->state == TASKSTATE_SUSPENDED ) return;

  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);
  tcb->state = TASKSTATE_SUSPENDED;
  mrbc_task_q_insert(tcb);
  mrbc_hal_enable_irq();

  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! resume the task

  @param  tcb		target task.
*/
void mrbc_resume_task(mrbc_tcb *tcb)
{
  if( tcb->state != TASKSTATE_SUSPENDED ) return;

  int flag_to_ready_state = (tcb->reason == 0);

  mrbc_hal_disable_irq();

  if( flag_to_ready_state ) preempt_running_task();

  mrbc_task_q_delete(tcb);
  tcb->state = flag_to_ready_state ? TASKSTATE_READY : TASKSTATE_WAITING;
  mrbc_task_q_insert(tcb);

  mrbc_hal_enable_irq();

  uint32_t wake;
  if( task_timed_wakeup(tcb, &wake) ) {
    if( (int32_t)(wake - wakeup_tick_) < 0 ) {
      wakeup_tick_ = wake;
    }
  }
}


//================================================================
/*! terminate the task.

  @param  tcb		target task.
  @note
    This API simply ends the task.
    note that this does not affect the lock status of mutex.
*/
void mrbc_terminate_task(mrbc_tcb *tcb)
{
  if( tcb->state == TASKSTATE_DORMANT ) return;

  terminate_task( tcb );
  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! join the task.

  @param  tcb		target task.
  @param  tcb_join	join task.
*/
void mrbc_join_task(mrbc_tcb *tcb, const mrbc_tcb *tcb_join)
{
  if( tcb->state == TASKSTATE_DORMANT ) return;
  if( tcb_join->state == TASKSTATE_DORMANT ) return;

  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);

  tcb->state    = TASKSTATE_WAITING;
  tcb->reason   = TASKREASON_JOIN;
  tcb->tcb_join = tcb_join;

  mrbc_task_q_insert(tcb);
  mrbc_hal_enable_irq();

  tcb->vm.flag_preemption = 1;
}



//================================================================
/*! mutex initialize

  @param  mutex		pointer to mrbc_mutex or NULL.
*/
mrbc_mutex * mrbc_mutex_init( mrbc_mutex *mutex )
{
  if( mutex == NULL ) {
    mutex = mrbc_raw_alloc( sizeof(mrbc_mutex) );
  }

  static const mrbc_mutex init_val = MRBC_MUTEX_INITIALIZER;
  *mutex = init_val;

  return mutex;
}


//================================================================
/*! mutex lock

  @param  mutex		pointer to mutex.
  @param  tcb		pointer to TCB.
*/
int mrbc_mutex_lock( mrbc_mutex *mutex, mrbc_tcb *tcb )
{
  MRBC_MUTEX_TRACE("mutex lock / MUTEX: %p TCB: %p",  mutex, tcb );

  int ret = 0;
  mrbc_hal_disable_irq();

  // Try lock mutex;
  if( mutex->lock == 0 ) {      // a future does use TAS?
    mutex->lock = 1;
    mutex->tcb = tcb;
    MRBC_MUTEX_TRACE("  lock OK\n" );
    goto DONE;
  }
  MRBC_MUTEX_TRACE("  lock FAIL\n" );

  // Can't lock mutex
  // check recursive lock.
  if( mutex->tcb == tcb ) {
    ret = 1;
    goto DONE;
  }

  // To WAITING state.
  mrbc_task_q_delete(tcb);
  tcb->state  = TASKSTATE_WAITING;
  tcb->reason = TASKREASON_MUTEX;
  tcb->mutex = mutex;
  mrbc_task_q_insert(tcb);
  tcb->vm.flag_preemption = 1;

 DONE:
  mrbc_hal_enable_irq();

  return ret;
}


//================================================================
/*! mutex unlock

  @param  mutex		pointer to mutex.
  @param  tcb		pointer to TCB.
*/
int mrbc_mutex_unlock( mrbc_mutex *mutex, mrbc_tcb *tcb )
{
  MRBC_MUTEX_TRACE("mutex unlock / MUTEX: %p TCB: %p\n",  mutex, tcb );

  // check some parameters.
  if( !mutex->lock ) return 1;
  if( mutex->tcb != tcb ) return 2;

  mrbc_hal_disable_irq();

  // wakeup ONE waiting task if exist.
  mrbc_tcb *tcb1;
  for( tcb1 = q_waiting_; tcb1 != NULL; tcb1 = tcb1->next ) {
    if( tcb1->reason == TASKREASON_MUTEX && tcb1->mutex == mutex ) break;
  }
  if( tcb1 ) {
    MRBC_MUTEX_TRACE("SW1: TCB: %p\n", tcb1 );
    mutex->tcb = tcb1;

    mrbc_task_q_delete(tcb1);
    tcb1->state = TASKSTATE_READY;
    tcb1->reason = 0;
    mrbc_task_q_insert(tcb1);

    preempt_running_task();
    goto DONE;
  }

  // find ONE mutex locked task in suspended queue.
  for( tcb1 = q_suspended_; tcb1 != NULL; tcb1 = tcb1->next ) {
    if( tcb1->reason == TASKREASON_MUTEX && tcb1->mutex == mutex ) break;
  }
  if( tcb1 ) {
    MRBC_MUTEX_TRACE("SW2: TCB: %p\n", tcb1 );
    mutex->tcb = tcb1;
    tcb1->reason = 0;
    goto DONE;
  }

  // other case, unlock mutex
  MRBC_MUTEX_TRACE("mutex unlock all.\n" );
  mutex->lock = 0;
  mutex->tcb = 0;

 DONE:
  mrbc_hal_enable_irq();

  return 0;
}


//================================================================
/*! mutex trylock

  @param  mutex		pointer to mutex.
  @param  tcb		pointer to TCB.
*/
int mrbc_mutex_trylock( mrbc_mutex *mutex, mrbc_tcb *tcb )
{
  MRBC_MUTEX_TRACE("mutex try lock / MUTEX: %p TCB: %p",  mutex, tcb );

  int ret;
  mrbc_hal_disable_irq();

  if( mutex->lock == 0 ) {
    mutex->lock = 1;
    mutex->tcb = tcb;
    ret = 0;
    MRBC_MUTEX_TRACE("  trylock OK\n" );
  }
  else {
    MRBC_MUTEX_TRACE("  trylock FAIL\n" );
    ret = 1;
  }

  mrbc_hal_enable_irq();
  return ret;
}


//================================================================
/*! clenaup all resources.

*/
void mrbc_cleanup(void)
{
  mrbc_cleanup_alloc();
  mrbc_cleanup_vm();
  mrbc_cleanup_symbol();

  memset( tcb_queue_, 0, sizeof(tcb_queue_) );
}


//================================================================
/*! (method) sleep for a specified number of seconds (CRuby compatible)

*/
static void c_sleep(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = mrbc_get_tcb(vm);

  if( argc == 0 ) {
    mrbc_suspend_task(tcb);
    return;
  }

  switch( mrbc_type(v[1]) ) {
  case MRBC_TT_INTEGER:
  {
    mrbc_int_t sec;
    sec = mrbc_integer(v[1]);
    SET_INT_RETURN(sec);
    mrbc_sleep_ms(tcb, sec * 1000);
    break;
  }

#if MRBC_USE_FLOAT
  case MRBC_TT_FLOAT:
  {
    mrbc_float_t sec;
    sec = mrbc_float(v[1]);
    SET_INT_RETURN(sec);
    mrbc_sleep_ms(tcb, (mrbc_int_t)(sec * 1000));
    break;
  }
#endif

  default:
    break;
  }
}


//================================================================
/*! (method) sleep for a specified number of milliseconds.

*/
static void c_sleep_ms(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = mrbc_get_tcb(vm);

  mrbc_int_t msec = mrbc_integer(v[1]);
  SET_INT_RETURN(msec);
  mrbc_sleep_ms(tcb, msec);
}



/*
  Task class
*/
//----------------------------------------------------------------
static mrbc_value sub_task_get(mrbc_vm *vm, mrbc_tcb *tcb)
{
  mrbc_value ret;

  // Only one instance is allocated.
  if( tcb->task_instance ) {
    ret = mrbc_immediate_value(MRBC_TT_OBJECT, .instance = tcb->task_instance );
  } else {
    ret = mrbc_instance_new(vm, MRBC_CLASS(Task), sizeof(mrbc_tcb *));
    *MRBC_INSTANCE_DATA_PTR( &ret, mrbc_tcb *) = tcb;
    tcb->task_instance = ret.instance;
  }

  mrbc_incref(&ret);
  return ret;
}


//================================================================
/*! (method) get task

  Task.get()           -> Task
  Task.get("TaskName") -> Task|nil
*/
static void c_task_get(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = NULL;

  if( mrbc_type(v[0]) != MRBC_TT_CLASS ) goto RETURN_NIL;

  // in case of Task.get()
  if( argc == 0 ) {
    tcb = mrbc_get_tcb(vm);
  }

#if MRBC_USE_STRING
  // in case of Task.get("TaskName")
  else if( mrbc_type(v[1]) == MRBC_TT_STRING ) {
    tcb = mrbc_find_task( mrbc_string_cstr( &v[1] ) );
  }
#endif

  if( tcb ) {
    mrbc_value ret = sub_task_get(vm, tcb);
    SET_RETURN(ret);
    return;             // normal return.
  }

 RETURN_NIL:
  SET_NIL_RETURN();
}


//================================================================
/*! (method) task list

  Task.list() -> Array[Task]
*/
static void c_task_list(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_array_new(vm, 1);

  mrbc_hal_disable_irq();

  for( int i = 0; i < NUM_TCB_QUEUE; i++ ) {
    for( mrbc_tcb *tcb = tcb_queue_[i]; tcb != NULL; tcb = tcb->next ) {
      mrbc_value task = sub_task_get(vm, tcb);
      mrbc_array_push( &ret, &task );
    }
  }

  mrbc_hal_enable_irq();

  SET_RETURN(ret);
}


#if MRBC_USE_STRING
//================================================================
/*! (method) task name list

  Task.name_list() -> Array[String]
*/
static void c_task_name_list(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_array_new(vm, 1);

  mrbc_hal_disable_irq();

  for( int i = 0; i < NUM_TCB_QUEUE; i++ ) {
    for( mrbc_tcb *tcb = tcb_queue_[i]; tcb != NULL; tcb = tcb->next ) {
      mrbc_value s = mrbc_string_new_cstr(vm, tcb->name);
      mrbc_array_push( &ret, &s );
    }
  }

  mrbc_hal_enable_irq();

  SET_RETURN(ret);
}


//================================================================
/*! (method) name setter.

  task.name = "Name"
  probably be used as Task.current.name = "MyName"
*/
static void c_task_set_name(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb;

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    DEPRECATED("Task.name=");
    tcb = mrbc_get_tcb(vm);
  } else {
    tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  }

  if( mrbc_type(v[1]) != MRBC_TT_STRING ) {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  mrbc_set_task_name( tcb, mrbc_string_cstr(&v[1]) );

  mrbc_incref( &v[1] );
  SET_RETURN( v[1] );
}


//================================================================
/*! (method) name getter

  Task.name() -> String    # get current task name
  task.name() -> String
*/
static void c_task_name(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value ret;

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    DEPRECATED("Task.name");
    ret = mrbc_string_new_cstr( vm, mrbc_get_tcb(vm)->name );
  } else {
    mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
    ret = mrbc_string_new_cstr(vm, tcb->name );
  }

  SET_RETURN(ret);
}
#endif


//================================================================
/*! (method) task priority setter

  Task.priority = n  # n = 0(high) .. 255(low)
  task.priority = n
*/
static void c_task_set_priority(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb;

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    DEPRECATED("Task.priority=");
    tcb = mrbc_get_tcb(vm);
  } else {
    tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  }

  if( mrbc_type(v[1]) != MRBC_TT_INTEGER ) {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }
  int n = mrbc_integer( v[1] );
  if( n < 0 || n > 255 ) {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  mrbc_change_priority( tcb, n );

  SET_RETURN( v[1] );
}


//================================================================
/*! (method) task priority getter

  task.priority() -> Integer
*/
static void c_task_priority(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb;

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    DEPRECATED("Task.priority");
    tcb = mrbc_get_tcb(vm);
  } else {
    tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  }

  SET_INT_RETURN( tcb->priority );
}


//================================================================
/*! (method) status

  task.status() -> String
*/
#if MRBC_USE_STRING
static void c_task_status(mrbc_vm *vm, mrbc_value v[], int argc)
{
  static const char *status_name[] =
    { "DORMANT", "READY", "WAITING ", "", "SUSPENDED" };
  static const char *reason_name[] =
    { "", "SLEEP", "MUTEX", "", "JOIN" };

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;

  const mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  mrbc_value ret = mrbc_string_new_cstr( vm, status_name[tcb->state / 2] );

  if( tcb->state == TASKSTATE_WAITING ) {
    mrbc_string_append_cstr( &ret, reason_name[tcb->reason] );
  }

  SET_RETURN(ret);
}
#endif


//================================================================
/*! (method) suspend task

  Task.suspend()        # suspend current task.
  task.suspend()        # suspend other task.
*/
static void c_task_suspend(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb;

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    tcb = mrbc_get_tcb(vm);
  } else {
    tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  }

  mrbc_suspend_task(tcb);
}


//================================================================
/*! (method) resume task

  task.resume()
*/
static void c_task_resume(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;

  mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);

  mrbc_resume_task(tcb);
}


//================================================================
/*! (method) terminate task

  task.terminate()
*/
static void c_task_terminate(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb;

  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    tcb = mrbc_get_tcb(vm);
  } else {
    tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  }

  mrbc_terminate_task(tcb);
}


//================================================================
/*! (method) raises an exception in the task.

  task.raise()
  task.raise( RangeError.new("message here!") )
*/
static void c_task_raise(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;
  mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);

  mrbc_vm *vm1 = &tcb->vm;
  mrbc_value exc;

  if( argc == 0 ) {
    exc = mrbc_exception_new( vm1, MRBC_CLASS(RuntimeError), 0, 0 );
  } else if( mrbc_type(v[1]) == MRBC_TT_EXCEPTION ) {
    exc = v[1];
    mrbc_incref(&exc);
  } else {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  mrbc_decref(&vm1->exception);
  vm1->exception = exc;
  vm1->flag_preemption = 2;

  if( tcb->state == TASKSTATE_WAITING && tcb->reason == TASKREASON_SLEEP ) {
    void mrbc_wakeup_task(mrbc_tcb *tcb);
    mrbc_wakeup_task( tcb );
  }
}


//================================================================
/*! (method) Waits for task to complete.

  task.join() -> Task
*/
static void c_task_join(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;

  mrbc_tcb *tcb_me = mrbc_get_tcb(vm);
  mrbc_tcb *tcb_join = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);

  mrbc_join_task(tcb_me, tcb_join);
}


//================================================================
/*! (method) returns task termination value.

  task.value
*/
static void c_task_value(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;

  mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);

  if( tcb->state != TASKSTATE_DORMANT ) {
    mrbc_raise(vm, 0, "task must be end");
    return;
  }

  mrbc_incref( &tcb->vm.regs[0] );
  SET_RETURN( tcb->vm.regs[0] );
}


//================================================================
/*! (method) pass execution to another task.

  Task.pass()
*/
static void c_task_pass(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) != MRBC_TT_CLASS ) return;

  mrbc_tcb *tcb = mrbc_get_tcb(vm);
  mrbc_relinquish(tcb);
}


//================================================================
/*! (method) create a task dynamically.

  Task.create( byte_code, regs_size = nil ) -> Task
*/
#if MRBC_USE_STRING
static void c_task_create(mrbc_vm *vm, mrbc_value v[], int argc)
{
  const char *byte_code;
  int regs_size = MAX_REGS_SIZE;

  // check argument.
  if( mrbc_type(v[0]) != MRBC_TT_CLASS ) goto ERROR_ARGUMENT;

  if( argc >= 1 && mrbc_type(v[1]) != MRBC_TT_STRING ) goto ERROR_ARGUMENT;
  mrbc_incref( &v[1] );
  byte_code = mrbc_string_cstr(&v[1]);

  if( argc >= 2 ) {
    if( mrbc_type(v[2]) != MRBC_TT_INTEGER ) goto ERROR_ARGUMENT;
    regs_size = mrbc_integer(v[2]);
  }

  // create TCB
  mrbc_tcb *tcb = mrbc_tcb_new( regs_size, TASKSTATE_DORMANT, MRBC_TASK_DEFAULT_PRIORITY );
  tcb->vm.flag_permanence = 1;

  if( !mrbc_create_task( byte_code, tcb ) ) return;

  // create Instance
  mrbc_value ret = mrbc_instance_new(vm, v[0].cls, sizeof(mrbc_tcb *));
  *MRBC_INSTANCE_DATA_PTR( &ret, mrbc_tcb *) = tcb;
  SET_RETURN( ret );
  return;

 ERROR_ARGUMENT:
  mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
}
#endif


//================================================================
/*! (method) start execution for a task.

  task.run
*/
static void c_task_run(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;

  mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  if( tcb->state != TASKSTATE_DORMANT ) return;

  mrbc_start_task(tcb);
}


//================================================================
/*! (method) reset the task execution state.

  task.rewind
*/
static void c_task_rewind(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) return;

  mrbc_tcb *tcb = *MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_tcb *);
  if( tcb->state != TASKSTATE_DORMANT ) return;

  mrbc_vm_begin( &tcb->vm );
}


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Task")
  FILE("_autogen_class_rrt0.h")

  METHOD( "get", c_task_get )
  METHOD( "current", c_task_get )
  METHOD( "list", c_task_list )
#if MRBC_USE_STRING
  METHOD( "name_list", c_task_name_list )
  METHOD( "name=", c_task_set_name )
  METHOD( "name", c_task_name )
#endif
  METHOD( "priority=", c_task_set_priority )
  METHOD( "priority", c_task_priority )
#if MRBC_USE_STRING
  METHOD( "status", c_task_status )
#endif

  METHOD( "suspend", c_task_suspend )
  METHOD( "resume", c_task_resume )
  METHOD( "terminate", c_task_terminate )
  METHOD( "raise", c_task_raise )

  METHOD( "join", c_task_join )
  METHOD( "value", c_task_value )
  METHOD( "pass", c_task_pass )

#if MRBC_USE_STRING
  METHOD( "create", c_task_create )
#endif
  METHOD( "run", c_task_run )
  METHOD( "rewind", c_task_rewind )
*/


/*
  Mutex class
*/
//================================================================
/*! (method) mutex constructor

*/
static void c_mutex_new(mrbc_vm *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_instance_new(vm, v[0].cls, sizeof(mrbc_mutex));

  mrbc_mutex *mutex = MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_mutex);

  mrbc_mutex_init( mutex );
}


//================================================================
/*! (method) mutex lock

*/
static void c_mutex_lock(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_mutex *mutex = MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_mutex);
  int res = mrbc_mutex_lock(mutex, mrbc_get_tcb(vm));
  if( res == 0 ) return;  // return self

  // raise ThreadError
  assert(!"Mutex recursive lock.");
}


//================================================================
/*! (method) mutex unlock

*/
static void c_mutex_unlock(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_mutex *mutex = MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_mutex);
  int res = mrbc_mutex_unlock( mutex, mrbc_get_tcb(vm));
  if( res == 0 ) return;  // return self

  // raise ThreadError
  assert(!"Mutex unlock error. not owner or not locked.");
}


//================================================================
/*! (method) mutex trylock

*/
static void c_mutex_trylock(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_mutex *mutex = MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_mutex);
  int res = mrbc_mutex_trylock( mutex, mrbc_get_tcb(vm));
  SET_BOOL_RETURN( res == 0 );
}


//================================================================
/*! (method) mutex locked?

*/
static void c_mutex_locked(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_mutex *mutex = MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_mutex);
  SET_BOOL_RETURN( mutex->lock != 0 );
}


//================================================================
/*! (method) mutex owned?

*/
static void c_mutex_owned(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_mutex *mutex = MRBC_INSTANCE_DATA_PTR(&v[0], mrbc_mutex);
  SET_BOOL_RETURN( mutex->lock != 0 && mutex->tcb == mrbc_get_tcb(vm) );
}


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Mutex")
  APPEND("_autogen_class_rrt0.h")

  METHOD( "new", c_mutex_new  )
  METHOD( "lock", c_mutex_lock  )
  METHOD( "unlock", c_mutex_unlock  )
  METHOD( "try_lock", c_mutex_trylock  )
  METHOD( "locked?", c_mutex_locked  )
  METHOD( "owned?", c_mutex_owned  )
*/



//================================================================
/*! (method) get tick counter
*/
static void c_vm_tick(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_INT_RETURN(tick_);
}

/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("VM")
  APPEND("_autogen_class_rrt0.h")

  METHOD( "tick", c_vm_tick )
*/
#include "_autogen_class_rrt0.h"



//================================================================
/*! initialize

  @param  heap_ptr	heap memory buffer.
  @param  size		its size.
*/
void mrbc_init(void *heap_ptr, unsigned int size)
{
  static uint8_t flag_hal_init_called = 0;

  if( !flag_hal_init_called ) {
    mrbc_hal_init();
    flag_hal_init_called = 1;
  }

  mrbc_init_alloc(heap_ptr, size);
  mrbc_init_global();
  mrbc_init_class_c();

  // Initialize included classes
  static mrbc_class * const rrt0_cls[] = {
    MRBC_CLASS(Task), MRBC_CLASS(Mutex), MRBC_CLASS(VM)
  };
  mrbc_value vcls = mrbc_immediate_value(MRBC_TT_CLASS);

  for( int i = 0; i < sizeof(rrt0_cls)/sizeof(rrt0_cls[0]); i++ ) {
    mrbc_class *cls = rrt0_cls[i];

    cls->super = MRBC_CLASS(Object);
    cls->method_link = 0;
    vcls.cls = cls;

    mrbc_set_const( vcls.cls->sym_id, &vcls );
  }

  mrbc_define_method(0, 0, "sleep", c_sleep);
  mrbc_define_method(0, 0, "sleep_ms", c_sleep_ms);

  mrbc_init_task_queue();

  mrbc_init_class_mrblib();
}



#ifdef MRBC_DEBUG
//================================================================
/*! DEBUG print queue

  (examples)
  void pqall(void);
  mrbc_define_method(0,0,"pqall", (mrbc_func_t)pqall);
 */
void pq(const mrbc_tcb *p_tcb)
{
  if( p_tcb == NULL ) return;

  // vm_id, TCB, name
  for( const mrbc_tcb *t = p_tcb; t; t = t->next ) {
    mrbc_printf("%d:%08x %-8.8s ", t->vm.vm_id, MRBC_PTR_TO_UINT32(t),
                t->name[0] ? t->name : "(noname)" );
  }
  mrbc_printf("\n");

#if 0
  // next ptr
  for( const mrbc_tcb *t = p_tcb; t; t = t->next ) {
    mrbc_printf(" next:%04x          ", (uint16_t)MRBC_PTR_TO_UINT32(t->next));
  }
  mrbc_printf("\n");
#endif

  // task priority, state.
  //  st:SsRr
  //     ^ suspended -> S:suspended
  //      ^ waiting  -> s:sleep m:mutex j:join q:queue
  //       ^ ready   -> R:ready
  //        ^ running-> r:running
  for( const mrbc_tcb *t = p_tcb; t; t = t->next ) {
    mrbc_printf(" pri:%3d", t->priority_preemption);
#if 1
    mrbc_tcb t1 = *t;               // Copy the value at this timing.
    mrbc_printf(" st:%c%c%c%c    ",
      (t1.state & TASKSTATE_SUSPENDED)?'S':'-',
      (t1.state & TASKSTATE_SUSPENDED)? ("-SM!J!!!Q"[t1.reason]) :
      (t1.state & TASKSTATE_WAITING)?   ("!sm!j!!!q"[t1.reason]) : '-',
      (t1.state & 0x02)?'R':'-',
      (t1.state & 0x01)?'r':'-' );
#else
    mrbc_printf(" s%04b r%04b ", t->state, t->reason);
#endif
  }
  mrbc_printf("\n");

  // timeslice, vm->flag_preemption, wakeup tick
  for( const mrbc_tcb *t = p_tcb; t; t = t->next ) {
    mrbc_printf(" ts:%-2d fp:%d ", t->timeslice, t->vm.flag_preemption);
    if( t->reason & TASKREASON_SLEEP ) {
      mrbc_printf("w:%-6d", t->wakeup_tick );
    } else {
      mrbc_printf("w:--    ");
    }
  }
  mrbc_printf("\n");

  // join task
  for( const mrbc_tcb *t = p_tcb; t; t = t->next ) {
    if( t->reason & TASKREASON_JOIN ) {
      mrbc_printf(" join:%p      ", t->tcb_join);
    } else {
      mrbc_printf("                    ");
    }
  }
  mrbc_printf("\n");

}

void pqall(void)
{
  mrbc_hal_disable_irq();
  mrbc_printf("<< tick_ = %d, wakeup_tick_ = %d >>\n", tick_, wakeup_tick_);
  mrbc_printf("<<<<< DORMANT >>>>>\n");   pq(q_dormant_);
  mrbc_printf("<<<<< READY >>>>>\n");     pq(q_ready_);
  mrbc_printf("<<<<< WAITING >>>>>\n");   pq(q_waiting_);
  mrbc_printf("<<<<< SUSPENDED >>>>>\n"); pq(q_suspended_);
  mrbc_hal_enable_irq();
}
#endif

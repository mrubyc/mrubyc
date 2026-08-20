/*! @file
  @brief
  Task::Queue for mruby/c

  <pre>
  Copyright (C) 2026-      HASUMI Hitoshi

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/*
  Unique sentinel returned by __pop_try when the current task is parked to
  WAITING. It is never exposed to Ruby; identity is checked in C by __retry?.
*/
static mrbc_value wait_retry_;

/*
  Unique sentinel returned by __pop_try when a timeout elapsed before an item
  became available. Never exposed to Ruby; identity is checked by __timeout?.
*/
static mrbc_value wait_timeout_;

/* Task::Error class, raised on illegal queue operations. */
static struct RClass *task_error_class_;

/* Cached instance variable symbols. */
static mrbc_sym sym_items_;
static mrbc_sym sym_closed_;

/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/

//================================================================
/*! Wake the highest-priority task waiting on this queue.

  @param  q	Queue instance the tasks are waiting on.
  @return	Non-zero if a task was woken.
*/
static int queue_wake_one_waiter(void *q)
{
  int woke = 0;

  mrbc_hal_disable_irq();
  for( mrbc_tcb *tcb = mrbc_task_q_waiting_head(); tcb != NULL; tcb = tcb->next ) {
    if( tcb->reason == TASKREASON_QUEUE && tcb->queue.target == q ) {
      mrbc_task_q_delete(tcb);
      tcb->state  = TASKSTATE_READY;
      tcb->reason = 0;
      tcb->queue.target      = NULL;
      tcb->queue.wakeup_tick = MRBC_WAIT_FOREVER;
      mrbc_task_q_insert(tcb);
      woke = 1;
      break;
    }
  }
  mrbc_hal_enable_irq();

  return woke;
}


//================================================================
/*! Wake all tasks waiting on this queue (used by close).

  @param  q	Queue instance the tasks are waiting on.
  @return	Non-zero if any task was woken.
*/
static int queue_wake_all_waiters(void *q)
{
  int woke = 0;

  mrbc_hal_disable_irq();
  mrbc_tcb *tcb = mrbc_task_q_waiting_head();
  while( tcb != NULL ) {
    mrbc_tcb *next = tcb->next;		// capture before the list is modified.
    if( tcb->reason == TASKREASON_QUEUE && tcb->queue.target == q ) {
      mrbc_task_q_delete(tcb);
      tcb->state  = TASKSTATE_READY;
      tcb->reason = 0;
      tcb->queue.target      = NULL;
      tcb->queue.wakeup_tick = MRBC_WAIT_FOREVER;
      mrbc_task_q_insert(tcb);
      woke = 1;
    }
    tcb = next;
  }
  mrbc_hal_enable_irq();

  return woke;
}


//================================================================
/*! Check whether the queue has been closed.

  (NOTE)
  @closed only ever holds true or false, so the decref is a no-op in practice.
  It is kept to stay paired with the incref done by mrbc_instance_getiv().

  @param  queue	Task::Queue instance.
  @return	Non-zero if the queue is closed.
*/
static int queue_is_closed(mrbc_value *queue)
{
  mrbc_value closed = mrbc_instance_getiv(queue, sym_closed_);
  int is_closed = (mrbc_type(closed) == MRBC_TT_TRUE);
  mrbc_decref(&closed);

  return is_closed;
}


//================================================================
/*! (method) initialize
*/
static void c_task_queue_initialize(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value items = mrbc_array_new(vm, 0);
  mrbc_instance_setiv(&v[0], sym_items_, &items);
  mrbc_decref(&items);

  mrbc_value closed = mrbc_false_value();
  mrbc_instance_setiv(&v[0], sym_closed_, &closed);
}


//================================================================
/*! (method) __push
*/
static void c_task_queue_push(mrbc_vm *vm, mrbc_value v[], int argc)
{
  switch( mrbc_task_queue_push(&v[0], &v[1]) ) {
  case MRBC_TASK_QUEUE_PUSH_OK:
    break;

  case MRBC_TASK_QUEUE_PUSH_OK_WOKE:
    // hand control back so the woken task is selected by the scheduler.
    mrbc_get_tcb(vm)->vm.flag_preemption = 1;
    break;

  case MRBC_TASK_QUEUE_PUSH_CLOSED:
    mrbc_raise(vm, task_error_class_, "queue closed");
    break;

  case MRBC_TASK_QUEUE_PUSH_INVALID:
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "invalid queue");
    break;
  }
}


//================================================================
/*! (method) __pop_try

  Tries to pop one item. Called as __pop_try(non_block, deadline) where
  deadline is an absolute tick value (from __deadline) or nil for no timeout.
  Returns:
    - the item if available
    - nil if closed and empty
    - raises Task::Error if non_block and empty
    - the WAIT_TIMEOUT sentinel if the deadline has already passed
    - the WAIT_RETRY sentinel if the current task was parked to WAITING

  The Ruby-level pop loops while __retry? is true, and maps WAIT_TIMEOUT to nil.
*/
static void c_task_queue_pop_try(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int non_block = (argc >= 1 && mrbc_type(v[1]) == MRBC_TT_TRUE);
  int has_timeout = (argc >= 2 && mrbc_type(v[2]) == MRBC_TT_INTEGER);
  uint32_t deadline = has_timeout ? (uint32_t)mrbc_integer(v[2]) : MRBC_WAIT_FOREVER;

  mrbc_value items = mrbc_instance_getiv(&v[0], sym_items_);

  // item available - return it.
  if( mrbc_array_size(&items) > 0 ) {
    mrbc_value ret = mrbc_array_shift(&items);
    mrbc_decref(&items);
    SET_RETURN(ret);
    return;
  }
  mrbc_decref(&items);

  // closed and empty.
  if( queue_is_closed(&v[0]) ) {
    SET_NIL_RETURN();
    return;
  }

  // non-blocking and empty.
  if( non_block ) {
    mrbc_raise(vm, task_error_class_, "queue empty");
    return;
  }

  // timeout already elapsed (covers timeout_ms: 0) - give up without parking.
  if( has_timeout && mrbc_deadline_reached(deadline) ) {
    mrbc_incref(&wait_timeout_);
    SET_RETURN(wait_timeout_);
    return;
  }

  // blocking: move the current task to WAITING and hand control back.
  mrbc_tcb *tcb = mrbc_get_tcb(vm);
  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);
  tcb->state  = TASKSTATE_WAITING;
  tcb->reason = TASKREASON_QUEUE;
  tcb->queue.target      = v[0].instance;
  tcb->queue.wakeup_tick = deadline;		// MRBC_WAIT_FOREVER when no timeout.
  if( has_timeout ) mrbc_register_wakeup(deadline);
  mrbc_task_q_insert(tcb);
  mrbc_hal_enable_irq();
  tcb->vm.flag_preemption = 1;

  // Return the hidden sentinel; the Ruby pop loop retries after wakeup.
  mrbc_incref(&wait_retry_);
  SET_RETURN(wait_retry_);
}


//================================================================
/*! (method) __retry?

  Returns true only if the argument is the WAIT_RETRY sentinel itself
  (pointer identity). Used by the Ruby pop loop.
*/
static void c_task_queue_is_wait_retry(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int r = (mrbc_type(v[1]) == MRBC_TT_OBJECT &&
           v[1].instance == wait_retry_.instance);
  SET_BOOL_RETURN(r);
}


//================================================================
/*! (method) __timeout?

  Returns true only if the argument is the WAIT_TIMEOUT sentinel itself
  (pointer identity). Used by the Ruby pop loop to map a timeout to nil.
*/
static void c_task_queue_is_wait_timeout(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int r = (mrbc_type(v[1]) == MRBC_TT_OBJECT &&
           v[1].instance == wait_timeout_.instance);
  SET_BOOL_RETURN(r);
}


//================================================================
/*! (method) __deadline

  Converts a millisecond timeout into an absolute tick deadline that the pop
  loop passes back to __pop_try on every retry. Computing it once in Ruby
  keeps the deadline fixed across spurious wakeups (e.g. a push that loses the
  item to a higher-priority task), so the total wait never drifts past the
  requested timeout. The argument is already validated as a non-negative
  Integer by Queue#pop.
*/
static void c_task_queue_deadline(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int overflow = 0;
  uint32_t deadline = mrbc_deadline_after_ms(mrbc_integer(v[1]), &overflow);
  if( overflow ) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "timeout_ms is too large");
    return;
  }
  SET_INT_RETURN((mrbc_int_t)deadline);
}


//================================================================
/*! (method) size
*/
static void c_task_queue_size(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value items = mrbc_instance_getiv(&v[0], sym_items_);
  int n = mrbc_array_size(&items);
  mrbc_decref(&items);
  SET_INT_RETURN(n);
}


//================================================================
/*! (method) empty?
*/
static void c_task_queue_empty_q(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value items = mrbc_instance_getiv(&v[0], sym_items_);
  int empty = (mrbc_array_size(&items) == 0);
  mrbc_decref(&items);
  SET_BOOL_RETURN(empty);
}


//================================================================
/*! (method) clear
*/
static void c_task_queue_clear(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value items = mrbc_instance_getiv(&v[0], sym_items_);
  mrbc_array_clear(&items);
  mrbc_decref(&items);
  // returns self.
}


//================================================================
/*! (method) close
*/
static void c_task_queue_close(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( !queue_is_closed(&v[0]) ) {
    mrbc_value t = mrbc_true_value();
    mrbc_instance_setiv(&v[0], sym_closed_, &t);
    if( queue_wake_all_waiters(v[0].instance) ) {
      mrbc_get_tcb(vm)->vm.flag_preemption = 1;
    }
  }
  // returns self.
}


//================================================================
/*! (method) closed?
*/
static void c_task_queue_closed_q(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_BOOL_RETURN( queue_is_closed(&v[0]) );
}


//================================================================
/*! (method) num_waiting
*/
static void c_task_queue_num_waiting(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int count = 0;

  mrbc_hal_disable_irq();
  for( mrbc_tcb *tcb = mrbc_task_q_waiting_head(); tcb != NULL; tcb = tcb->next ) {
    if( tcb->reason == TASKREASON_QUEUE && tcb->queue.target == v[0].instance ) {
      count++;
    }
  }
  mrbc_hal_enable_irq();

  SET_INT_RETURN(count);
}


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Task::Queue")
  FILE("_autogen_class_task_queue.h")

  METHOD( "initialize", c_task_queue_initialize )
  METHOD( "__push", c_task_queue_push )
  METHOD( "__pop_try", c_task_queue_pop_try )
  METHOD( "__retry?", c_task_queue_is_wait_retry )
  METHOD( "__timeout?", c_task_queue_is_wait_timeout )
  METHOD( "__deadline", c_task_queue_deadline )
  METHOD( "size", c_task_queue_size )
  METHOD( "length", c_task_queue_size )
  METHOD( "empty?", c_task_queue_empty_q )
  METHOD( "clear", c_task_queue_clear )
  METHOD( "close", c_task_queue_close )
  METHOD( "closed?", c_task_queue_closed_q )
  METHOD( "num_waiting", c_task_queue_num_waiting )
*/
#include "_autogen_class_task_queue.h"


/***** Global functions *****************************************************/

//================================================================
/*! initialize
*/
void mrbc_init_task_queue(void)
{
  // Register Task::Queue (builtin class) as a constant under Task.
  mrbc_value vcls = mrbc_immediate_value(MRBC_TT_CLASS, .cls = MRBC_CLASS(Task_Queue));
  mrbc_set_class_const(MRBC_CLASS(Task), MRBC_SYM(Queue), &vcls);

  // Define Task::Error < StandardError.
  task_error_class_ = mrbc_define_class_under(0, MRBC_CLASS(Task), "Error",
                                              MRBC_CLASS(StandardError));

  // Cache instance variable symbols.
  sym_items_  = mrbc_str_to_symid("@items");
  sym_closed_ = mrbc_str_to_symid("@closed");

  // Create the unique, private sentinels (kept for the process life).
  wait_retry_   = mrbc_instance_new(0, MRBC_CLASS(Object), 0);
  wait_timeout_ = mrbc_instance_new(0, MRBC_CLASS(Object), 0);
}


//================================================================
/*! Push a value into a Task::Queue from C and wake one waiting task.

  Ownership of value stays with the caller; the queue takes its own reference.
  An invalid receiver and a closed queue are reported as a result code rather
  than an exception, so this is usable where no VM context is at hand.

  (NOTE)
  This grows the item array through the mruby/c allocator and edits the task
  queues, so it must NOT be called from an interrupt handler, nor from any
  context that could re-enter the VM. There is no portable way to detect that
  at run time, so the caller is responsible for honouring it. To feed a queue
  from an interrupt, buffer the data in an ISR-safe ring buffer and push it
  from a task.

  A caller running inside the VM must set flag_preemption when
  MRBC_TASK_QUEUE_PUSH_OK_WOKE is returned, otherwise the woken task waits for
  the current time slice to expire. See c_task_queue_push().

  @param  queue	Task::Queue instance (or an instance of its subclass).
  @param  value	value to push.
  @return	result code. see mrbc_task_queue_push_result.
*/
mrbc_task_queue_push_result mrbc_task_queue_push(mrbc_value *queue, mrbc_value *value)
{
  if( mrbc_type(*queue) != MRBC_TT_OBJECT ||
      !mrbc_obj_is_kind_of(queue, MRBC_CLASS(Task_Queue)) ) {
    return MRBC_TASK_QUEUE_PUSH_INVALID;
  }

  if( queue_is_closed(queue) ) return MRBC_TASK_QUEUE_PUSH_CLOSED;

  mrbc_value items = mrbc_instance_getiv(queue, sym_items_);
  assert( mrbc_type(items) == MRBC_TT_ARRAY );
  mrbc_incref(value);
  mrbc_array_push(&items, value);
  mrbc_decref(&items);

  return queue_wake_one_waiter(queue->instance) ?
         MRBC_TASK_QUEUE_PUSH_OK_WOKE : MRBC_TASK_QUEUE_PUSH_OK;
}

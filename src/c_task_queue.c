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
    if( tcb->reason == TASKREASON_QUEUE && tcb->queue == q ) {
      mrbc_task_q_delete(tcb);
      tcb->state  = TASKSTATE_READY;
      tcb->reason = 0;
      tcb->queue  = NULL;
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
    if( tcb->reason == TASKREASON_QUEUE && tcb->queue == q ) {
      mrbc_task_q_delete(tcb);
      tcb->state  = TASKSTATE_READY;
      tcb->reason = 0;
      tcb->queue  = NULL;
      mrbc_task_q_insert(tcb);
      woke = 1;
    }
    tcb = next;
  }
  mrbc_hal_enable_irq();

  return woke;
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
  mrbc_value closed = mrbc_instance_getiv(&v[0], sym_closed_);
  int is_closed = (mrbc_type(closed) == MRBC_TT_TRUE);
  mrbc_decref(&closed);
  if( is_closed ) {
    mrbc_raise(vm, task_error_class_, "queue closed");
    return;
  }

  mrbc_value items = mrbc_instance_getiv(&v[0], sym_items_);
  mrbc_array_push(&items, &v[1]);
  mrbc_set_tt(&v[1], MRBC_TT_EMPTY);	// ownership moved into the array.
  mrbc_decref(&items);

  if( queue_wake_one_waiter(v[0].instance) ) {
    mrbc_get_tcb(vm)->vm.flag_preemption = 1;
  }
}


//================================================================
/*! (method) __pop_try

  Tries to pop one item. Returns:
    - the item if available
    - nil if closed and empty
    - raises Task::Error if non_block and empty
    - the WAIT_RETRY sentinel if the current task was parked to WAITING

  The Ruby-level pop loops while __retry? is true.
*/
static void c_task_queue_pop_try(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int non_block = (argc >= 1 && mrbc_type(v[1]) == MRBC_TT_TRUE);

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
  mrbc_value closed = mrbc_instance_getiv(&v[0], sym_closed_);
  int is_closed = (mrbc_type(closed) == MRBC_TT_TRUE);
  mrbc_decref(&closed);
  if( is_closed ) {
    SET_NIL_RETURN();
    return;
  }

  // non-blocking and empty.
  if( non_block ) {
    mrbc_raise(vm, task_error_class_, "queue empty");
    return;
  }

  // blocking: move the current task to WAITING and hand control back.
  mrbc_tcb *tcb = mrbc_get_tcb(vm);
  mrbc_hal_disable_irq();
  mrbc_task_q_delete(tcb);
  tcb->state  = TASKSTATE_WAITING;
  tcb->reason = TASKREASON_QUEUE;
  tcb->queue  = v[0].instance;
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
  mrbc_value closed = mrbc_instance_getiv(&v[0], sym_closed_);
  int was_closed = (mrbc_type(closed) == MRBC_TT_TRUE);
  mrbc_decref(&closed);

  if( !was_closed ) {
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
  mrbc_value closed = mrbc_instance_getiv(&v[0], sym_closed_);
  int is_closed = (mrbc_type(closed) == MRBC_TT_TRUE);
  mrbc_decref(&closed);
  SET_BOOL_RETURN(is_closed);
}


//================================================================
/*! (method) num_waiting
*/
static void c_task_queue_num_waiting(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int count = 0;

  mrbc_hal_disable_irq();
  for( mrbc_tcb *tcb = mrbc_task_q_waiting_head(); tcb != NULL; tcb = tcb->next ) {
    if( tcb->reason == TASKREASON_QUEUE && tcb->queue == v[0].instance ) {
      count++;
    }
  }
  mrbc_hal_enable_irq();

  SET_INT_RETURN(count);
}


/***** Global functions *****************************************************/

//================================================================
/*! initialize
*/
void mrbc_init_task_queue(mrbc_class *task_class)
{
  // Register Task::Queue (builtin class) as a constant under Task.
  mrbc_value vcls = mrbc_immediate_value(MRBC_TT_CLASS, .cls = MRBC_CLASS(Task_Queue));
  mrbc_set_class_const(task_class, mrbc_str_to_symid("Queue"), &vcls);

  // Define Task::Error < StandardError.
  task_error_class_ = mrbc_define_class_under(0, task_class, "Error",
                                              MRBC_CLASS(StandardError));

  // Cache instance variable symbols.
  sym_items_  = mrbc_str_to_symid("@items");
  sym_closed_ = mrbc_str_to_symid("@closed");

  // Create the unique, private WAIT_RETRY sentinel (kept for the process life).
  wait_retry_ = mrbc_instance_new(0, MRBC_CLASS(Object), 0);
}


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Task::Queue")
  FILE("_autogen_class_task_queue.h")

  METHOD( "initialize", c_task_queue_initialize )
  METHOD( "__push", c_task_queue_push )
  METHOD( "__pop_try", c_task_queue_pop_try )
  METHOD( "__retry?", c_task_queue_is_wait_retry )
  METHOD( "size", c_task_queue_size )
  METHOD( "length", c_task_queue_size )
  METHOD( "empty?", c_task_queue_empty_q )
  METHOD( "clear", c_task_queue_clear )
  METHOD( "close", c_task_queue_close )
  METHOD( "closed?", c_task_queue_closed_q )
  METHOD( "num_waiting", c_task_queue_num_waiting )
*/
#include "_autogen_class_task_queue.h"

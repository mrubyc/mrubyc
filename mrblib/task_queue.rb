class Task
  class Queue
    # The WAIT_RETRY sentinel and its identity check (__retry?) live in
    # c_task_queue.c. The sentinel is never exposed to Ruby.

    def push(obj)
      __push(obj)
      self
    end
    alias enq push
    alias << push

    # Blocks until an item is available (default), or raises if non_block is true.
    # Returns nil if the queue is closed and empty.
    #
    # The loop is not a busy-wait. When __pop_try finds the queue empty it moves
    # the current task to WAITING and sets flag_preemption=1 before returning the
    # WAIT_RETRY sentinel. The VM detects flag_preemption=1 at the next opcode
    # boundary and exits mrbc_vm_run, handing control back to the scheduler. This
    # task does not run again until a push (or close) moves it back to READY. The
    # loop body therefore executes at most once per wakeup event.
    def pop(non_block = false)
      while true
        v = __pop_try(non_block)
        return v unless __retry?(v)
      end
    end
    alias deq pop
    alias shift pop
  end
end

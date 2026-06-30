
class TaskQueueTest < Picotest::Test

  # Only synchronous behaviour is covered here: pushing, non-blocking pop
  # (pop(true)), sizing, clearing and closing. The blocking pop / wakeup
  # paths need multiple tasks and are tested separately.

  description "new creates a queue"
  def test_new
    q = Task::Queue.new
    assert q.is_a?(Task::Queue)
  end

  description "push and non-blocking pop return items in FIFO order"
  def test_push_pop_fifo
    q = Task::Queue.new
    q.push(1)
    q.push(2)
    q.push(3)
    assert_equal 1, q.pop(true)
    assert_equal 2, q.pop(true)
    assert_equal 3, q.pop(true)
  end

  description "<< alias"
  def test_shovel_alias
    q = Task::Queue.new
    q << :a
    q << :b
    assert_equal :a, q.pop(true)
    assert_equal :b, q.pop(true)
  end

  description "enq / deq aliases"
  def test_enq_deq_alias
    q = Task::Queue.new
    q.enq(10)
    assert_equal 10, q.deq(true)
  end

  description "shift alias"
  def test_shift_alias
    q = Task::Queue.new
    q.push(:x)
    assert_equal :x, q.shift(true)
  end

  description "size and length"
  def test_size_and_length
    q = Task::Queue.new
    assert_equal 0, q.size
    assert_equal 0, q.length
    q.push(1)
    assert_equal 1, q.size
    q.push(2)
    assert_equal 2, q.length
    q.pop(true)
    assert_equal 1, q.size
  end

  description "empty?"
  def test_empty
    q = Task::Queue.new
    assert q.empty?
    q.push(1)
    assert_false q.empty?
    q.pop(true)
    assert q.empty?
  end

  description "clear"
  def test_clear
    q = Task::Queue.new
    q.push(1)
    q.push(2)
    q.clear
    assert q.empty?
    assert_equal 0, q.size
  end

  description "pop(true) raises Task::Error when empty"
  def test_pop_nonblock_empty_raises
    q = Task::Queue.new
    assert_raise(Task::Error) { q.pop(true) }
  end

  description "close and closed?"
  def test_close_and_closed
    q = Task::Queue.new
    assert_false q.closed?
    q.close
    assert q.closed?
  end

  description "push raises Task::Error after close"
  def test_push_after_close_raises
    q = Task::Queue.new
    q.close
    assert_raise(Task::Error) { q.push(1) }
  end

  description "pop(true) returns nil when closed and empty"
  def test_pop_nonblock_closed_empty_nil
    q = Task::Queue.new
    q.close
    assert_nil q.pop(true)
  end

  description "pops remaining items after close, then nil"
  def test_pop_remaining_after_close
    q = Task::Queue.new
    q.push(1)
    q.push(2)
    q.close
    assert_equal 1, q.pop(true)
    assert_equal 2, q.pop(true)
    assert_nil q.pop(true)
  end

  description "double close is a no-op"
  def test_double_close
    q = Task::Queue.new
    q.close
    q.close
    assert q.closed?
  end

  description "num_waiting is 0 with no blocked tasks"
  def test_num_waiting_zero
    q = Task::Queue.new
    assert_equal 0, q.num_waiting
  end

  # The blocking timeout_ms behaviour (returning nil after the deadline, or the
  # item when a push arrives first) needs the scheduler and multiple tasks, so
  # it is exercised separately. Argument validation happens before any blocking
  # and is checked here.

  description "pop with zero timeout returns an available item, else nil"
  def test_pop_zero_timeout_nonblocking
    q = Task::Queue.new
    q.push(:ready)
    assert_equal :ready, q.pop(timeout_ms: 0)
    assert_nil q.pop(timeout_ms: 0)
  end

  description "pop with non-Integer timeout raises TypeError"
  def test_pop_timeout_type_error
    q = Task::Queue.new
    assert_raise(TypeError) { q.pop(timeout_ms: 1.0) }
  end

  description "pop with negative timeout raises ArgumentError"
  def test_pop_timeout_negative
    q = Task::Queue.new
    assert_raise(ArgumentError) { q.pop(timeout_ms: -1) }
  end

  description "pop with non_block and timeout raises ArgumentError"
  def test_pop_timeout_with_non_block
    q = Task::Queue.new
    assert_raise(ArgumentError) { q.pop(true, timeout_ms: 1) }
  end
end

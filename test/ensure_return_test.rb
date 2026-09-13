class EnsureReturn
  # A method with a frame of its own, so a call from an ensure clause
  # lays registers past the end of the caller's frame.
  def helper
    a = 1
    b = 2
    [a, b].size
  end

  # The ensure clause calls a Ruby method while a return is pending.
  def return_then_call
    begin
      return 42
    ensure
      helper
    end
  end

  # The ensure clause rewrites the local the return took its value from.
  def return_local_then_rewrite
    x = 1
    begin
      return x
    ensure
      x = 2
    end
  end

  # Two ensure clauses on the way out, each calling a method.
  def return_through_two_ensures(log)
    begin
      begin
        return "inner"
      ensure
        log << "A#{helper}"
      end
    ensure
      log << "B#{helper}"
    end
  end

  # A method called from the ensure returns through an ensure itself.
  def return_nested(log)
    begin
      return :outer
    ensure
      log << return_then_call
    end
  end

  # A raise in the ensure clause replaces the pending return.
  def return_then_raise
    begin
      return :lost
    ensure
      raise "replaced"
    end
  end

  # The value is an object, not an immediate.
  def return_object
    s = "str" + "ing"
    begin
      return s
    ensure
      helper
    end
  end
end


class EnsureReturnTest < Picotest::Test

  def setup
    @obj = EnsureReturn.new
  end

  description "return value survives a method call in the ensure clause"
  def test_return_then_call
    assert_equal 42, @obj.return_then_call
  end

  description "return value is taken before the ensure clause runs"
  def test_return_local_then_rewrite
    assert_equal 1, @obj.return_local_then_rewrite
  end

  description "return value survives two ensure clauses"
  def test_return_through_two_ensures
    log = []
    assert_equal "inner", @obj.return_through_two_ensures(log)
    assert_equal ["A2", "B2"], log
  end

  description "a return pending in the ensure clause does not clobber the outer one"
  def test_return_nested
    log = []
    assert_equal :outer, @obj.return_nested(log)
    assert_equal [42], log
  end

  description "a raise in the ensure clause replaces the pending return"
  def test_return_then_raise
    assert_raise(RuntimeError) { @obj.return_then_raise }
  end

  description "an object return value survives the ensure clause"
  def test_return_object
    assert_equal "string", @obj.return_object
  end

  description "$! is nil in an ensure clause entered normally"
  def test_errinfo_on_normal_entry
    seen = :unset
    begin
      1
    ensure
      seen = $!
    end
    assert_nil seen
  end

  description "$! names the exception in an ensure clause it unwinds through"
  def test_errinfo_on_exception
    seen = nil
    begin
      begin
        raise "boom"
      ensure
        seen = $!
      end
    rescue => e
    end
    assert_equal RuntimeError, seen.class
    assert_equal "boom", seen.message
    assert_equal e.message, seen.message
  end

  description "$! is nil in an ensure clause a break passes through"
  def test_errinfo_on_break
    seen = :unset
    i = 0
    while true
      begin
        i += 1
        break if 2 <= i
      ensure
        seen = $!
      end
    end
    assert_nil seen
    assert_equal 2, i
  end
end

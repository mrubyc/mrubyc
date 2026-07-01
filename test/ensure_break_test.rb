class EnsureBreak
  def yielder
    yield 1
    yield 2
  end

  # Implicit return: the method value is the last expression (`result`).
  # `break` escapes the block; the ensure clause must not clobber the value.
  def implicit_return
    result = nil
    yielder do |x|
      result = "got#{x}"
      break
    end
    result
  ensure
    @ensured = true
  end

  # Explicit return combined with the same block break + ensure.
  def explicit_return
    result = nil
    yielder do |x|
      result = "got#{x}"
      break
    end
    return result
  ensure
    @ensured = true
  end

  # The break itself carries the value.
  def break_value
    yielder do |x|
      break "broke#{x}"
    end
  ensure
    @ensured = true
  end
end


class EnsureBreakTest < Picotest::Test

  def setup
    @obj = EnsureBreak.new
  end

  description "break out of block then implicit return with ensure"
  def test_implicit_return
    assert_equal "got1", @obj.implicit_return
  end

  description "break out of block then explicit return with ensure"
  def test_explicit_return
    assert_equal "got1", @obj.explicit_return
  end

  description "break value survives the ensure clause"
  def test_break_value
    assert_equal "broke1", @obj.break_value
  end
end

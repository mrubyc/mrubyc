class EnsureRetry
  # Retries twice, then succeeds.
  def retry_then_success(log)
    count = 0
    begin
      count += 1
      log << "try#{count}"
      raise "again" if count < 3
      :done
    rescue
      retry
    ensure
      log << "ensure#{count}"
    end
  end

  # Retries twice, then returns from the rescue clause.
  def retry_then_return(log)
    count = 0
    begin
      count += 1
      raise "again"
    rescue
      retry if count < 3
      return :gave_up
    ensure
      log << "ensure#{count}"
    end
  end

  # The retry is in an inner begin that starts at the same position
  # as the outer begin with the ensure clause.
  def retry_in_inner_begin(log)
    count = 0
    begin
      begin
        count += 1
        raise "again" if count < 3
        :done
      rescue
        retry
      end
    ensure
      log << "ensure#{count}"
    end
  end
end


class EnsureRetryTest < Picotest::Test

  def setup
    @obj = EnsureRetry.new
  end

  description "retry does not run the ensure clause"
  def test_retry_then_success
    log = []
    assert_equal :done, @obj.retry_then_success(log)
    assert_equal ["try1", "try2", "try3", "ensure3"], log
  end

  description "the ensure clause runs once after retries and a return"
  def test_retry_then_return
    log = []
    assert_equal :gave_up, @obj.retry_then_return(log)
    assert_equal ["ensure3"], log
  end

  description "retry in an inner begin does not run the outer ensure clause"
  def test_retry_in_inner_begin
    log = []
    assert_equal :done, @obj.retry_in_inner_begin(log)
    assert_equal ["ensure3"], log
  end
end

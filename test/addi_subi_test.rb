class AddiSubiTest < Picotest::Test

  class AddiSubiTestClass
  end

  description "In general, Object#+ should raise NoMethodError"
  def test_no_method_error
    # PicoRuby's Time#+ and Time#- that take Integer as an argument should work.
    # Bue we cannot test them in mruby/c
    assert_raise(NoMethodError) do
      AddiSubiTestClass.new + 1
    end
    assert_raise(NoMethodError) do
      AddiSubiTestClass.new - 1
    end
  end

  description "OP_ADDI and OP_SUBI should in Integer and Float"
  def test_addi_subi
    assert_equal 3,   2 + 1
    assert_equal 3.1, 2.1 + 1
    assert_equal 1,   2 - 1
    assert_equal 1.1, 2.1 - 1
  end
end


class ConstTest < Picotest::Test

  class A
    CONST = 1
    class B
      def should_raise
        puts B::CONST
      end
    end
  end

  description 'Should raise NameError'
  def test_name_error
    assert_equal 1, A::CONST
    b = A::B.new
    assert_raise(NameError) do
      b.should_raise
    end
  end

end


class ClassTest1 < Picotest::Test

  module A
    class B
      def self.hello
        C.hello
      end
    end

    class C
      def self.hello
        return 'Hello from C'
      end
    end
  end

  class MyClass < A::B
  end

  def test_test1
    assert_equal('Hello from C', A::B.hello)
  end

  def test_test2
    assert_equal('Hello from C', MyClass.hello)
  end

  def test_object_constants
    assert Object.constants.is_a?(Array)
  end

end

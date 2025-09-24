class MyBlock
  def initialize
    @result = Array.new
  end

  def func1
    yield
  end

  def each_double(array)
    array.each do |v|
      double(v)
    end
  end

  def double(val)
    @result << val * 2
  end

  def result
    @result
  end
end

class MyBlockTest < Picotest::Test

  def setup
    @obj = MyBlock.new
  end

  description 'basic_yield'
  def test_yeald
    $count = 0
    @obj.func1 { $count += 1 }
    assert_equal 1, $count
  end

  description "instance method inside block"
  def test_instance_method
    @obj.each_double([1, 2, 3])
    assert_equal [2, 4, 6], @obj.result
  end
end


class MyBlockInit
  attr_reader :a

  def initialize
    @a = ["a", "b"].map{|row| row }
  end
end


class MyBlockInitTest < Picotest::Test

  description "use block in initializer method"
  def test_block_init
    obj = MyBlockInit.new
    assert_equal( ["a", "b"], obj.a )
  end

end

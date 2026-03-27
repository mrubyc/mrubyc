
class MySuperTest < Picotest::Test

  class MySuper0
    attr_reader :a1, :a2

    def initialize( a1, a2 )
      @a1 = a1
      @a2 = a2
    end

    def method1( a1, a2 )
      @a1 = a1
      @a2 = a2
    end
  end

  class MySuper1 < MySuper0
    def initialize( a1 = 1, a2 = 2 )
      super
    end

    def method1( a1, a2 )
      a1 *= 2
      super( a1, a2*2 )
    end
  end

  class MySuper2 < MySuper1
    def method1( a1, a2 )
      super
      @a2 = 2222
    end
  end

  description "MySuper1"
  def test_super_1
    obj = MySuper1.new()
    assert_equal 1, obj.a1
    assert_equal 2, obj.a2

    obj.method1( 11, 22 )
    assert_equal 22, obj.a1
    assert_equal 44, obj.a2
  end

  description "MySuper2"
  def test_super_2
    obj = MySuper2.new()
    obj.method1( 11, 22 )
    assert_equal 22, obj.a1
    assert_equal 2222, obj.a2
  end

  # super with rest parameters

  class MySuperRest0
    attr_reader :name, :args

    def initialize( name, *args )
      @name = name
      @args = args
    end

    def method1( prefix, *words )
      [prefix, *words].join(" ")
    end
  end

  class MySuperRest1 < MySuperRest0
    def initialize( name, *args )
      super
    end

    def method1( prefix, *words )
      super
    end
  end

  description "super with rest params (initialize)"
  def test_super_rest_init
    obj = MySuperRest1.new("Rex", "a", "b")
    assert_equal "Rex", obj.name
    assert_equal ["a", "b"], obj.args
  end

  description "super with rest params (no extra args)"
  def test_super_rest_empty
    obj = MySuperRest1.new("Buddy")
    assert_equal "Buddy", obj.name
    assert_equal [], obj.args
  end

  description "super with rest params (method)"
  def test_super_rest_method
    obj = MySuperRest1.new("Rex")
    assert_equal "hello world foo", obj.method1("hello", "world", "foo")
  end

  # super with optional + rest parameters

  class MySuperOptRest0
    attr_reader :a, :b, :rest

    def initialize( a, b = 10, *rest )
      @a = a
      @b = b
      @rest = rest
    end
  end

  class MySuperOptRest1 < MySuperOptRest0
    def initialize( a, b = 10, *rest )
      super
    end
  end

  description "super with optional + rest params"
  def test_super_opt_rest
    obj = MySuperOptRest1.new(1, 2, 3, 4)
    assert_equal 1, obj.a
    assert_equal 2, obj.b
    assert_equal [3, 4], obj.rest
  end

  description "super with optional + rest params (defaults)"
  def test_super_opt_rest_defaults
    obj = MySuperOptRest1.new(1)
    assert_equal 1, obj.a
    assert_equal 10, obj.b
    assert_equal [], obj.rest
  end

  # super with rest params inside a block

  class MySuperBlock0
    attr_reader :args
    def initialize( *args )
      @args = args
    end
  end

  class MySuperBlock1 < MySuperBlock0
    def initialize( *args )
      [1].each do
        super
      end
    end
  end

  description "super with rest params inside a block"
  def test_super_rest_block
    obj = MySuperBlock1.new("x", "y")
    assert_equal ["x", "y"], obj.args
  end

  description "super with empty rest params inside a block"
  def test_super_rest_block_empty
    obj = MySuperBlock1.new
    assert_equal [], obj.args
  end

end


class OpBasicTest < Picotest::Test

  description "op_add"
  def test_op_add
    a = 1
    b = 2
    assert_equal( a + b, 3 )

    a = 1
    b = 2.0
    assert_equal( a + b, 3.0 )

    a = 1.0
    b = 2
    assert_equal( a + b, 3.0 )

    a = 1.0
    b = 2.0
    assert_equal( a + b, 3.0 )
  end

  description "op_addi"
  def test_op_addi
    a = 1
    assert_equal( a + 2, 3 )
    assert_equal( 1 + 2, 3 )

    a = 1.0
    assert_equal( a + 2, 3.0 )
    assert_equal( 1.0 + 2, 3.0 )
  end

  description "op_sub"
  def test_op_sub
    a = 1
    b = 2
    assert_equal( a - b, -1 )

    a = 1
    b = 2.0
    assert_equal( a - b, -1.0 )

    a = 1.0
    b = 2
    assert_equal( a - b, -1.0 )

    a = 1.0
    b = 2.0
    assert_equal( a - b, -1.0 )
  end

  description "op_subi"
  def test_op_subi
    a = 1
    assert_equal( a - 2, -1 )

    a = 1.0
    assert_equal( a - 2, -1.0 )
  end

  description "op_mul"
  def test_op_mul
    a = 2
    b = 3
    assert_equal a * b, 6

    a = 2
    b = 3.0
    assert_equal a * b, 6.0

    a = 2.0
    b = 3
    assert_equal a * b, 6.0

    a = 2.0
    b = 3.0
    assert_equal a * b, 6.0
  end

  description "op_lt"
  def test_op_lt
    a = 1
    b = 2
    assert_true( a < b )
    assert_false( b < a )

    a = 1
    b = 2.0
    assert_true( a < b )
    assert_false( b < a )

    a = 1.0
    b = 2
    assert_true( a < b )
    assert_false( b < a )

    a = 1.0
    b = 2.0
    assert_true( a < b )
    assert_false( b < a )

    a = 1
    b = 1
    assert_false( a < b )

    a = 1
    b = 1.0
    assert_false( a < b )

    a = 1.0
    b = 1
    assert_false( a < b )

    a = 1.0
    b = 1.0
    assert_false( a < b )
  end

  description "op_le"
  def test_op_le
    a = 1
    b = 2
    assert_true( a <= b )
    assert_false( b <= a )

    a = 1
    b = 2.0
    assert_true( a <= b )
    assert_false( b <= a )

    a = 1.0
    b = 2
    assert_true( a <= b )
    assert_false( b <= a )

    a = 1.0
    b = 2.0
    assert_true( a <= b )
    assert_false( b <= a )

    a = 1
    b = 1
    assert_true( a <= b )

    a = 1
    b = 1.0
    assert_true( a <= b )

    a = 1.0
    b = 1
    assert_true( a <= b )

    a = 1.0
    b = 1.0
    assert_true( a <= b )
  end

  description "op_gt"
  def test_op_gt
    a = 1
    b = 2
    assert_false( a > b )
    assert_true( b > a )

    a = 1
    b = 2.0
    assert_false( a > b )
    assert_true( b > a )

    a = 1.0
    b = 2
    assert_false( a > b )
    assert_true( b > a )

    a = 1.0
    b = 2.0
    assert_false( a > b )
    assert_true( b > a )

    a = 1
    b = 1
    assert_false( a > b )

    a = 1
    b = 1.0
    assert_false( a > b )

    a = 1.0
    b = 1
    assert_false( a > b )

    a = 1.0
    b = 1.0
    assert_false( a > b )
  end

  description "op_ge"
  def test_op_ge
    a = 1
    b = 2
    assert_false( a >= b )
    assert_true( b >= a )

    a = 1
    b = 2.0
    assert_false( a >= b )
    assert_true( b >= a )

    a = 1.0
    b = 2
    assert_false( a >= b )
    assert_true( b >= a )

    a = 1.0
    b = 2.0
    assert_false( a >= b )
    assert_true( b >= a )

    a = 1
    b = 1
    assert_true( a >= b )

    a = 1
    b = 1.0
    assert_true( a >= b )

    a = 1.0
    b = 1
    assert_true( a >= b )

    a = 1.0
    b = 1.0
    assert_true( a >= b )
  end

  description "op_compare with a difference that does not fit in int"
  def test_op_compare_wide_difference
    a = 2000000000
    b = -2000000000
    assert_false( a < b )
    assert_false( a <= b )
    assert_true( a > b )
    assert_true( a >= b )
    assert_true( b < a )
    assert_equal( 1, a <=> b )
    assert_equal( -1, b <=> a )
  end

end

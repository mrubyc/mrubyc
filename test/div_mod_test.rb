class DivModTest < Picotest::Test

  description "マイナス値を含む割り算"
  def test_div
    # |a| > |b|
    # 割り切れる
    a,b =  12, 3
    assert_equal(  4, a / b )
    a,b = -12, 3
    assert_equal( -4, a / b )
    a,b =  12,-3
    assert_equal( -4, a / b )
    a,b = -12,-3
    assert_equal(  4, a / b )

    # 割り切れない
    a,b =  11, 3
    assert_equal(  3, a / b )
    a,b = -11, 3
    assert_equal( -4, a / b )
    a,b =  11,-3
    assert_equal( -4, a / b )
    a,b = -11,-3
    assert_equal(  3, a / b )

    a,b =  10, 3
    assert_equal(  3, a / b )
    a,b = -10, 3
    assert_equal( -4, a / b )
    a,b =  10,-3
    assert_equal( -4, a / b )
    a,b = -10,-3
    assert_equal(  3, a / b )


    # |a| == |b|
    # 割り切れる
    a,b =  3, 3
    assert_equal(  1, a / b )
    a,b = -3, 3
    assert_equal( -1, a / b )
    a,b =  3,-3
    assert_equal( -1, a / b )
    a,b = -3,-3
    assert_equal(  1, a / b )


    # |a| < |b|
    # 割り切れない
    a,b =  3, 10
    assert_equal(  0, a / b )
    a,b = -3, 10
    assert_equal( -1, a / b )
    a,b =  3,-10
    assert_equal( -1, a / b )
    a,b = -3,-10
    assert_equal(  0, a / b )


    # a == 0
    a,b =  0,  3
    assert_equal( 0, a / b )
    a,b =  0, -3
    assert_equal( 0, a / b )

    # b == 0
    a,b =  3,  0
    assert_raise( ZeroDivisionError ) { a / b }
    a,b = -3,  0
    assert_raise( ZeroDivisionError ) { a / b }

    # a == b == 0
    a,b =  0,  0
    assert_raise( ZeroDivisionError ) { a / b }
  end


  description "マイナス値を含む剰余"
  def test_mod
    # |a| > |b|
    # 割り切れる
    a,b =  12, 3
    assert_equal(  0, a % b )
    a,b = -12, 3
    assert_equal(  0, a % b )
    a,b =  12,-3
    assert_equal(  0, a % b )
    a,b = -12,-3
    assert_equal(  0, a % b )


    # 割り切れない
    a,b =  11, 3
    assert_equal(  2, a % b )
    a,b = -11, 3
    assert_equal(  1, a % b )
    a,b =  11,-3
    assert_equal( -1, a % b )
    a,b = -11,-3
    assert_equal( -2, a % b )

    a,b =  10, 3
    assert_equal(  1, a % b )
    a,b = -10, 3
    assert_equal(  2, a % b )
    a,b =  10,-3
    assert_equal( -2, a % b )
    a,b = -10,-3
    assert_equal( -1, a % b )


    # |a| == |b|
    # 割り切れる
    a,b =  3, 3
    assert_equal(  0, a % b )
    a,b = -3, 3
    assert_equal(  0, a % b )
    a,b =  3,-3
    assert_equal(  0, a % b )
    a,b = -3,-3
    assert_equal(  0, a % b )


    # |a| < |b|
    # 割り切れない
    a,b =  3, 10
    assert_equal(  3, a % b )
    a,b = -3, 10
    assert_equal(  7, a % b )
    a,b =  3,-10
    assert_equal( -7, a % b )
    a,b = -3,-10
    assert_equal( -3, a % b )


    # a == 0
    a,b =  0,  3
    assert_equal( 0, a % b )
    a,b =  0, -3
    assert_equal( 0, a % b )

    # b == 0
    a,b =  3,  0
    assert_raise( ZeroDivisionError ) { a % b }
    a,b = -3,  0
    assert_raise( ZeroDivisionError ) { a % b }

    # a == b == 0
    a,b =  0,  0
    assert_raise( ZeroDivisionError ) { a % b }
  end
end

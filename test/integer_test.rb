
class IntegerTest < Picotest::Test

  description "abs"
  def test_abs
    assert_equal 12, 12.abs
    assert_equal 34.56, (-34.56).abs
    assert_equal 34.56, -34.56.abs
  end

  description "chr"
  def test_chr
    assert_equal "A", 65.chr
    assert_equal 1, 0x7F.chr.bytesize

    # Values 128-255 must produce a single byte, not UTF-8 multi-byte
    assert_equal 1, 0x80.chr.bytesize
    assert_equal 1, 0xB5.chr.bytesize
    assert_equal 1, 0xFF.chr.bytesize
    assert_equal 0xB5, 0xB5.chr.getbyte(0)
    assert_equal 0xFF, 0xFF.chr.getbyte(0)

    # 256+ raises RangeError (same as CRuby without encoding argument)
    assert_raise(RangeError) { 256.chr }
    assert_raise(RangeError) { 0x1FF.chr }
  end

  description "times"
  def test_times
    result = 0
    10.times { |i|
      result += i
    }
    assert_equal 45, result
  end

  description "upto"
  def test_upto
    result = 0
    -1.upto 10 do |i|
      result += i
    end
    assert_equal 54, result
  end

  description "downto"
  def test_downto
    result = 0
    10.downto -1 do |i|
      result += i
    end
    assert_equal 54, result
  end

  description "to_f"
  def test_to_f
    assert_equal( 10.0, 10.to_f )
    assert_equal( -10.0, -10.to_f )
  end

  description "to_i"
  def test_to_i_
    assert_equal( 10, 10.to_i )
    assert_equal( -10, -10.to_i )
  end

  description "to_s"
  def test_to_s
    assert_equal( "10", 10.to_s )
    assert_equal( "1010", 10.to_s(2) )
    assert_equal( "12", 10.to_s(8) )
    assert_equal( "a", 10.to_s(16) )
    assert_equal( "z", 35.to_s(36) )

    assert_equal( "-1", -1.to_s )
    assert_equal( "-10", -10.to_s )
    assert_equal( "-15wx", -54321.to_s(36) )
  end

  description "clamp"
  def test_clamp
    assert_equal 2, 10.clamp(0, 2)
    assert_equal 2, 10.clamp(-1, 2)
    assert_equal -2, -10.clamp(-2, 2)
    assert_equal 2.0, 10.clamp(0, 2.0)
    assert_equal 2, 10.clamp(-1.0, 2.0)
    assert_equal -2, -10.clamp(-2.0, 2)
    assert_raise(ArgumentError, "min argument must be smaller than max argument") do
      0.clamp(1, -1)
    end
    assert_raise(ArgumentError, "wrong number of arguments") do
      0.clamp(1)
    end
    assert_raise(ArgumentError, "wrong number of arguments") do
      0.clamp(1..2)
    end
    assert_raise(ArgumentError, "comparison failed") do
      0.clamp("1", "2")
    end
    assert_raise(ArgumentError, "comparison failed") do
      0.clamp(0..10, 9)
    end
  end

end

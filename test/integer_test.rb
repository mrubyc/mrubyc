# frozen_string_literal: true

class IntegerTest < MrubycTestCase

  description "abs"
  def abs_case
    assert_equal 12, 12.abs
    assert_equal 34.56, (-34.56).abs
    assert_equal 34.56, -34.56.abs
  end

  description "chr"
  def chr_case
    assert_equal "\u0000", 0.chr
    assert_equal "A", 65.chr
    # NOTE: Pre-processing in CRuby will result in an error if no arguments are added.
    assert_equal "ã", 227.chr('UTF-8')
    assert_equal "あ", 12354.chr('UTF-8')
    assert_equal "𩸽", 171581.chr('UTF-8')
    assert_equal "\u{10FFFF}", 0x10FFFF.chr('UTF-8')

    assert_raise(RangeError.new("invalid codepoint in UTF-8")) do
      0xD800.chr('UTF-8')
    end

    assert_raise(RangeError.new("invalid codepoint in UTF-8")) do
      0xDFFF.chr('UTF-8')
    end

    assert_raise(RangeError.new("out of char range")) do
      0x110000.chr('UTF-8')
    end
  end

  description "times"
  def times_case
    result = 0
    10.times { |i|
      result += i
    }
    assert_equal 45, result
  end

  description "upto"
  def upto_case
    result = 0
    -1.upto 10 do |i|
      result += i
    end
    assert_equal 54, result
  end

  description "downto"
  def downto_case
    result = 0
    10.downto -1 do |i|
      result += i
    end
    assert_equal 54, result
  end

  description "to_f"
  def to_f_case
    assert_equal( 10.0, 10.to_f )
    assert_equal( -10.0, -10.to_f )
  end

  description "to_i"
  def to_i__case
    assert_equal( 10, 10.to_i )
    assert_equal( -10, -10.to_i )
  end

  description "to_s"
  def to_s_case
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
  def clamp_case
    assert_equal 2, 10.clamp(0, 2)
    assert_equal 2, 10.clamp(-1, 2)
    assert_equal -2, -10.clamp(-2, 2)
    assert_equal 2.0, 10.clamp(0, 2.0)
    assert_equal 2, 10.clamp(-1.0, 2.0)
    assert_equal -2, -10.clamp(-2.0, 2)
    assert_raise(ArgumentError.new("min argument must be smaller than max argument")) do
      0.clamp(1, -1)
    end
    assert_raise(ArgumentError.new("wrong number of arguments (expected 2)")) do
      0.clamp(1)
    end
    assert_raise(ArgumentError.new("wrong number of arguments (expected 2)")) do
      0.clamp(1..2)
    end
    assert_raise(ArgumentError.new("comparison failed")) do
      0.clamp("1", "2")
    end
    assert_raise(ArgumentError.new("comparison failed")) do
      0.clamp(0..10, 9)
    end
  end

end

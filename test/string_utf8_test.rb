# coding: utf-8
# UTF-8 String Tests
# These tests verify UTF-8 string handling when MRBC_USE_STRING_UTF8=1
#
# Test strings used:
# - ASCII: "hello" (5 bytes, 5 chars)
# - Japanese hiragana: "あいう" (9 bytes, 3 chars) - each char is 3 bytes
# - Mixed: "aあb" (5 bytes, 3 chars)
# - Emoji: "😀" (4 bytes, 1 char)
# - Chinese: "中文" (6 bytes, 2 chars)

class StringUtf8Test < Picotest::Test

  #
  # String#size / String#length
  #
  description "size returns character count for ASCII"
  def test_size_ascii
    assert_equal 5, "hello".size
    assert_equal 5, "hello".length
  end

  description "size returns character count for Japanese"
  def test_size_japanese
    # "あいう" is 3 characters (9 bytes in UTF-8)
    assert_equal 3, "あいう".size
    assert_equal 3, "あいう".length
  end

  description "size returns character count for mixed"
  def test_size_mixed
    # "aあb" is 3 characters (5 bytes)
    assert_equal 3, "aあb".size
  end

  description "size returns character count for emoji"
  def test_size_emoji
    # "😀" is 1 character (4 bytes)
    assert_equal 1, "😀".size
  end

  description "size returns character count for Chinese"
  def test_size_chinese
    # "中文" is 2 characters (6 bytes)
    assert_equal 2, "中文".size
  end

  description "empty string size is 0"
  def test_size_empty
    assert_equal 0, "".size
  end

  #
  # String#[] (slice)
  #
  description "slice with positive index returns UTF-8 character"
  def test_slice_positive_index
    s = "あいう"
    assert_equal "あ", s[0]
    assert_equal "い", s[1]
    assert_equal "う", s[2]
  end

  description "slice with negative index returns UTF-8 character"
  def test_slice_negative_index
    s = "あいう"
    assert_equal "う", s[-1]
    assert_equal "い", s[-2]
    assert_equal "あ", s[-3]
  end

  description "slice with out of range index returns nil"
  def test_slice_out_of_range
    s = "あいう"
    assert_equal nil, s[3]
    assert_equal nil, s[-4]
  end

  description "slice with index and length"
  def test_slice_index_length
    s = "あいうえお"
    assert_equal "あい", s[0, 2]
    assert_equal "いうえ", s[1, 3]
    assert_equal "えお", s[3, 2]
    assert_equal "お", s[4, 10]  # length exceeds string
  end

  description "slice with range"
  def test_slice_range
    s = "あいうえお"
    assert_equal "あいう", s[0..2]
    assert_equal "あい", s[0...2]
    assert_equal "いうえ", s[1..3]
    assert_equal "うえお", s[-3..-1]
  end

  description "slice mixed ASCII and multibyte"
  def test_slice_mixed
    s = "aあbいc"
    assert_equal "a", s[0]
    assert_equal "あ", s[1]
    assert_equal "b", s[2]
    assert_equal "い", s[3]
    assert_equal "c", s[4]
    assert_equal "aあb", s[0, 3]
  end

  #
  # String#[]= (insert/replace)
  #
  description "replace single UTF-8 character"
  def test_replace_single_char
    s = "あいう"
    s[1] = "X"
    assert_equal "あXう", s
  end

  description "replace with UTF-8 character"
  def test_replace_with_utf8
    s = "abc"
    s[1] = "あ"
    assert_equal "aあc", s
  end

  description "replace range of UTF-8 characters"
  def test_replace_range
    s = "あいうえお"
    s[1, 2] = "XY"
    assert_equal "あXYえお", s
  end

  description "replace with longer string"
  def test_replace_longer
    s = "あいう"
    s[1] = "XXXX"
    assert_equal "あXXXXう", s
  end

  description "replace with shorter string"
  def test_replace_shorter
    s = "あいうえお"
    s[1, 3] = "X"
    assert_equal "あXお", s
  end

  #
  # String#index
  #
  description "index finds UTF-8 substring and returns character index"
  def test_index_utf8
    s = "あいうえお"
    assert_equal 0, s.index("あ")
    assert_equal 1, s.index("い")
    assert_equal 2, s.index("う")
    assert_equal 3, s.index("えお")
  end

  description "index returns nil when not found"
  def test_index_not_found
    s = "あいう"
    assert_equal nil, s.index("か")
  end

  description "index with offset uses character index"
  def test_index_with_offset
    s = "あいあい"
    assert_equal 0, s.index("あ")
    assert_equal 2, s.index("あ", 1)
  end

  description "index in mixed string"
  def test_index_mixed
    s = "aあbいc"
    assert_equal 1, s.index("あ")
    assert_equal 3, s.index("い")
    assert_equal 2, s.index("b")
  end

  #
  # String#ord
  #
  description "ord returns Unicode codepoint for ASCII"
  def test_ord_ascii
    assert_equal 97, "a".ord
    assert_equal 65, "A".ord
  end

  description "ord returns Unicode codepoint for Japanese"
  def test_ord_japanese
    # "あ" is U+3042 = 12354
    assert_equal 12354, "あ".ord
    # "い" is U+3044 = 12356
    assert_equal 12356, "い".ord
  end

  description "ord returns Unicode codepoint for emoji"
  def test_ord_emoji
    # "😀" is U+1F600 = 128512
    assert_equal 128512, "😀".ord
  end

  description "ord returns first character codepoint"
  def test_ord_first_char
    assert_equal 12354, "あいう".ord  # returns codepoint of "あ"
  end

  #
  # Integer#chr
  #
  description "chr creates ASCII character"
  def test_chr_ascii
    assert_equal "a", 97.chr
    assert_equal "A", 65.chr
  end

  description "chr raises RangeError for codepoint > 255"
  def test_chr_out_of_range
    assert_raise(RangeError) { 256.chr }
    assert_raise(RangeError) { 12354.chr }
    assert_raise(RangeError) { 128512.chr }
  end

  #
  # String#slice!
  #
  description "slice! removes UTF-8 character by index"
  def test_slice_self_index
    s = "あいう"
    r = s.slice!(1)
    assert_equal "い", r
    assert_equal "あう", s
  end

  description "slice! removes range of UTF-8 characters"
  def test_slice_self_range
    s = "あいうえお"
    r = s.slice!(1, 2)
    assert_equal "いう", r
    assert_equal "あえお", s
  end

  description "slice! with negative index"
  def test_slice_self_negative
    s = "あいう"
    r = s.slice!(-1)
    assert_equal "う", r
    assert_equal "あい", s
  end

  #
  # String#split
  #
  description "split by empty string splits into UTF-8 characters"
  def test_split_empty
    s = "あいう"
    result = s.split("")
    assert_equal 3, result.size
    assert_equal "あ", result[0]
    assert_equal "い", result[1]
    assert_equal "う", result[2]
  end

  description "split mixed string by empty string"
  def test_split_empty_mixed
    s = "aあb"
    result = s.split("")
    assert_equal 3, result.size
    assert_equal "a", result[0]
    assert_equal "あ", result[1]
    assert_equal "b", result[2]
  end

  description "split by UTF-8 separator"
  def test_split_utf8_separator
    s = "あ,い,う"
    result = s.split(",")
    assert_equal 3, result.size
    assert_equal "あ", result[0]
    assert_equal "い", result[1]
    assert_equal "う", result[2]
  end

  #
  # String#chars
  #
  description "chars returns array of UTF-8 characters"
  def test_chars
    s = "あいう"
    result = s.chars
    assert_equal 3, result.size
    assert_equal "あ", result[0]
    assert_equal "い", result[1]
    assert_equal "う", result[2]
  end

  description "chars with mixed string"
  def test_chars_mixed
    s = "aあb"
    result = s.chars
    assert_equal 3, result.size
    assert_equal "a", result[0]
    assert_equal "あ", result[1]
    assert_equal "b", result[2]
  end

  description "chars with emoji"
  def test_chars_emoji
    s = "a😀b"
    result = s.chars
    assert_equal 3, result.size
    assert_equal "a", result[0]
    assert_equal "😀", result[1]
    assert_equal "b", result[2]
  end

  #
  # String#encoding
  #
  description "encoding returns UTF-8"
  def test_encoding
    assert_equal "UTF-8", "hello".encoding
    assert_equal "UTF-8", "あいう".encoding
  end

  #
  # String#valid_encoding?
  #
  description "valid_encoding? returns true for valid UTF-8"
  def test_valid_encoding_true
    assert_equal true, "hello".valid_encoding?
    assert_equal true, "あいう".valid_encoding?
    assert_equal true, "".valid_encoding?
    assert_equal true, "a😀b".valid_encoding?
  end

  #
  # String concatenation and operations
  #
  description "concatenation preserves UTF-8"
  def test_concat
    s = "あ" + "い"
    assert_equal "あい", s
    assert_equal 2, s.size
  end

  description "append preserves UTF-8"
  def test_append
    s = "あ"
    s << "い"
    assert_equal "あい", s
    assert_equal 2, s.size
  end

  description "multiply preserves UTF-8"
  def test_multiply
    s = "あ" * 3
    assert_equal "あああ", s
    assert_equal 3, s.size
  end

  #
  # Edge cases
  #
  description "empty string operations"
  def test_empty_string
    s = ""
    assert_equal 0, s.size
    assert_equal nil, s[0]
    assert_equal [], s.chars
    assert_equal true, s.valid_encoding?
  end

  description "single character strings"
  def test_single_char
    assert_equal 1, "あ".size
    assert_equal "あ", "あ"[0]
    assert_equal ["あ"], "あ".chars
  end

  #
  # String#bytesize
  #
  description "bytesize returns byte count for ASCII"
  def test_bytesize_ascii
    assert_equal 5, "hello".bytesize
  end

  description "bytesize returns byte count for Japanese"
  def test_bytesize_japanese
    # "あいう" is 9 bytes in UTF-8 (3 bytes each)
    assert_equal 9, "あいう".bytesize
  end

  description "bytesize returns byte count for mixed"
  def test_bytesize_mixed
    # "aあb" = 1 + 3 + 1 = 5 bytes
    assert_equal 5, "aあb".bytesize
  end

  description "bytesize returns byte count for emoji"
  def test_bytesize_emoji
    # "😀" is 4 bytes in UTF-8
    assert_equal 4, "😀".bytesize
  end

  description "bytesize differs from size for multibyte"
  def test_bytesize_vs_size
    s = "あいう"
    assert_equal 3, s.size        # 3 characters
    assert_equal 9, s.bytesize    # 9 bytes
  end

  #
  # String#reverse
  #
  description "reverse returns reversed ASCII string"
  def test_reverse_ascii
    assert_equal "olleh", "hello".reverse
  end

  description "reverse returns reversed Japanese string"
  def test_reverse_japanese
    assert_equal "語本日", "日本語".reverse
  end

  description "reverse handles mixed ASCII and multibyte"
  def test_reverse_mixed
    assert_equal "cあba", "abあc".reverse
  end

  description "reverse handles emoji"
  def test_reverse_emoji
    assert_equal "🎊🎉", "🎉🎊".reverse
  end

  description "reverse handles accented characters"
  def test_reverse_accented
    assert_equal "éfac", "café".reverse
  end

  description "reverse! modifies string in place"
  def test_reverse_self
    s = "あいう"
    s.reverse!
    assert_equal "ういあ", s
  end

  description "reverse! returns self"
  def test_reverse_self_returns_self
    s = "abc"
    result = s.reverse!
    assert_equal "cba", result
    assert_equal "cba", s
  end

  description "reverse empty string"
  def test_reverse_empty
    assert_equal "", "".reverse
  end

  description "reverse single character"
  def test_reverse_single_char
    assert_equal "あ", "あ".reverse
  end

  #
  # Outer BMP characters (4-byte UTF-8)
  #
  description "size counts 4-byte UTF-8 characters correctly"
  def test_size_outer_bmp
    # 𩸽 (U+29E3D) is outside BMP, 4 bytes in UTF-8
    assert_equal 1, "𩸽".size
  end

  description "slice works with 4-byte UTF-8 characters"
  def test_slice_outer_bmp
    s = "a𩸽b"
    assert_equal "a", s[0]
    assert_equal "𩸽", s[1]
    assert_equal "b", s[2]
    assert_equal 3, s.size
  end

  description "ord returns correct codepoint for 4-byte UTF-8"
  def test_ord_outer_bmp
    # 𩸽 is U+29E3D = 171581
    assert_equal 171581, "𩸽".ord
  end

  description "chr raises RangeError for 4-byte codepoint"
  def test_chr_outer_bmp_raises
    # U+29E3D = 171581
    assert_raise(RangeError) { 171581.chr }
  end

  description "reverse works with 4-byte UTF-8 characters"
  def test_reverse_outer_bmp
    assert_equal "ba𩸽", "𩸽ab".reverse
  end

  description "concatenation with 4-byte UTF-8 characters"
  def test_concat_outer_bmp
    s = "あ" + "𩸽"
    assert_equal "あ𩸽", s
    assert_equal 2, s.size
  end

  description "chars with 4-byte UTF-8 characters"
  def test_chars_outer_bmp
    s = "a𩸽b"
    result = s.chars
    assert_equal 3, result.size
    assert_equal "a", result[0]
    assert_equal "𩸽", result[1]
    assert_equal "b", result[2]
  end

  #
  # String#<< with codepoint
  #
  description "append codepoint creates UTF-8 character"
  def test_append_codepoint
    s = "a"
    s << 12354  # U+3042 = "あ"
    assert_equal "aあ", s
    assert_equal 2, s.size
  end

  description "append ASCII codepoint"
  def test_append_ascii_codepoint
    s = "a"
    s << 65  # "A"
    assert_equal "aA", s
  end

  description "append 4-byte codepoint"
  def test_append_outer_bmp_codepoint
    s = "a"
    s << 171581  # U+29E3D = "𩸽"
    assert_equal "a𩸽", s
    assert_equal 2, s.size
  end

  #
  # each_char
  #
  description "each_char iterates over UTF-8 characters"
  def test_each_char
    chars = []
    "あいう".each_char { |c| chars << c }
    assert_equal 3, chars.size
    assert_equal "あ", chars[0]
    assert_equal "い", chars[1]
    assert_equal "う", chars[2]
  end

  description "each_char with mixed content"
  def test_each_char_mixed
    chars = []
    "aあ𩸽".each_char { |c| chars << c }
    assert_equal 3, chars.size
    assert_equal "a", chars[0]
    assert_equal "あ", chars[1]
    assert_equal "𩸽", chars[2]
  end

  #
  # each_byte
  #
  description "each_byte iterates over bytes"
  def test_each_byte
    bytes = []
    "あ".each_byte { |b| bytes << b }
    # "あ" = E3 81 82 in UTF-8
    assert_equal 3, bytes.size
    assert_equal 0xE3, bytes[0]
    assert_equal 0x81, bytes[1]
    assert_equal 0x82, bytes[2]
  end

  #
  # tr with UTF-8
  #
  description "tr with UTF-8 characters"
  def test_tr_utf8
    assert_equal "あXう", "あいう".tr("い", "X")
  end

  description "tr! with UTF-8 characters"
  def test_tr_self_utf8
    s = "あいう"
    s.tr!("い", "X")
    assert_equal "あXう", s
  end

  #
  # upcase/downcase with UTF-8 (ASCII only by default)
  #
  description "upcase works with ASCII in UTF-8 string"
  def test_upcase_utf8_ascii
    assert_equal "ABCあいう", "abcあいう".upcase
  end

  description "downcase works with ASCII in UTF-8 string"
  def test_downcase_utf8_ascii
    assert_equal "abcあいう", "ABCあいう".downcase
  end

  #
  # String#ascii_only?
  #
  description "ascii_only? returns true for ASCII string"
  def test_ascii_only_true
    assert_equal true, "hello".ascii_only?
    assert_equal true, "".ascii_only?
    assert_equal true, "abc123!@#".ascii_only?
    assert_equal true, "\t\n\r".ascii_only?
    assert_equal true, "\x00\x7F".ascii_only?
  end

  description "ascii_only? returns false for multibyte string"
  def test_ascii_only_false
    assert_equal false, "あ".ascii_only?
    assert_equal false, "hello世界".ascii_only?
    assert_equal false, "café".ascii_only?
    assert_equal false, "😀".ascii_only?
    assert_equal false, "a𩸽b".ascii_only?
  end

  #
  # Invalid/incomplete UTF-8 sequences
  #
  description "size treats isolated leading byte as 1 character"
  def test_size_incomplete_utf8
    # 0xD1 alone is an incomplete 2-byte UTF-8 sequence (no continuation byte)
    # Each invalid byte must be treated as a single character
    assert_equal 1, "\xD1".size
    assert_equal 4, "\x04\x08i\xD1".size
  end

  description "slice of incomplete UTF-8 sequence returns 1-byte string"
  def test_slice_incomplete_utf8
    # "\x04\x08i\xD1": the last byte 0xD1 has no valid continuation byte
    # s[3] must return a 1-byte string, not read into the null terminator
    s = "\x04\x08i\xD1"
    c = s[3]
    assert_equal 1, c.bytesize
    assert_equal 209, c.ord  # 0xD1 = 209, not 0x440 = 1088
  end

  description "slice with isolated continuation byte returns 1-byte string"
  def test_slice_isolated_continuation_byte
    # 0x80 is a continuation byte with no leading byte - invalid UTF-8
    s = "a\x80b"
    assert_equal 3, s.size
    assert_equal "\x80", s[1]
    assert_equal 1, s[1].bytesize
  end

  description "byteslice is always byte-based even in UTF-8 mode"
  def test_byteslice_utf8
    # "\xC2\xA5" is UTF-8 for the yen sign (2 bytes, 1 char), followed by "abc"
    # Total: 5 bytes, 4 chars
    s = "\xC2\xA5abc"
    assert_equal 5, s.bytesize
    assert_equal 4, s.size
    assert_equal "\xC2", s.byteslice(0)
    assert_equal "\xC2\xA5", s.byteslice(0, 2)
    assert_equal "a", s.byteslice(2)
    assert_equal "abc", s.byteslice(2, 3)
  end

end

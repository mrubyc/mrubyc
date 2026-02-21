# coding: utf-8

class StringTest < Picotest::Test

  description "String.new with arg"
  def test_string_new_with_arg
    str = String.new("a string instance")
    assert_equal "a string instance", str
  end

  description "String.new without arg"
  def test_string_new_without_arg
    str = String.new
    assert_equal "", str
  end

  description "==, !="
  def test_op_eq
    assert_equal true, "abc" == "abc"
    assert_equal false, "abc" == "ABC"
    assert_equal false, "abc" == "abcd"
    assert_equal false, "abc" != "abc"
    assert_equal true, "abc" != "ABC"
  end

  description "self * times -> String"
  def test_mul
    s1 = "ABCDEFG"
    s2 = "0123456789"
    assert_equal "ABCDEFGABCDEFG", s1 * 2
    assert_equal "abcabc", "abc" * 2
    assert_equal "01234567890123456789", s2 * 2
  end

  description "self + other -> String"
  def test_add
    s1 = "ABCDEFG"
    s2 = "0123456789"
    assert_equal "ABCDEFG0123456789", s1 + s2
    assert_equal "ABCDEFG123", s1 + "123"
    assert_equal "abc0123456789", "abc" + s2
    assert_equal "abc123", "abc" + "123"
  end

  description "self << other -> self"
  def test_addi
    s1 = "ABCDEFG"
    s2 = "0123456789"
    s1 << s2
    assert_equal "ABCDEFG0123456789", s1
    s1 << "abc"
    assert_equal "ABCDEFG0123456789abc", s1
    assert_equal "abcdef", "abc" << "def"
    s1 << 65
    assert_equal "ABCDEFG0123456789abcA", s1
  end

  description "self <=> other -> (minus) | 0 | (plus)"
  def test_compare
    assert ("aaa" <=> "xxx") < 0
    assert ("aaa" <=> "aaa") == 0
    assert ("xxx" <=> "aaa") > 0
    assert ("string" <=> "stringAA") < 0
    assert ("string" <=> "string") == 0
    assert ("stringAA" <=> "string") > 0
  end

  description "self == other -> bool"
  def test_op_eq_2
    s1 = "ABCDEFG"
    assert_equal "ABCDEFG", s1
  end

  description "self[nth] -> String | nil"
  def test_nth
    assert_equal "r", 'bar'[2]
    assert_equal true, 'bar'[2] == ?r
    assert_equal "r", 'bar'[-1]
    assert_equal nil, 'bar'[3]
    assert_equal nil, 'bar'[-4]
  end

  description "self[nth, len] -> String | nil"
  def test_nth_len
    str0 = "bar"
    assert_equal "r", str0[2, 1]
    assert_equal "",  str0[2, 0]
    assert_equal "r", str0[2, 100]  # (右側を超えても平気
    assert_equal "r", str0[-1, 1]
    assert_equal "r", str0[-1, 2]   # (あくまでも「右に向かって len 文字」

    assert_equal "", str0[3, 1]
    assert_equal nil, str0[4, 1]
    assert_equal nil, str0[-4, 1]
    str1 = str0[0, 2]               # (str0 の「一部」を str1 とする
    assert_equal "ba", str1
    str1[0] = "XYZ"
    assert_equal "XYZa", str1     #(str1 の内容が破壊的に変更された
    assert_equal "bar", str0      #(str0 は無傷、 str1 は str0 と内容を共有していない
  end

  description "self[nth]=val -> val"
  def test_nth_return_val
    str = "bar"
    assert_equal "B", str[0] = "B"
    assert_equal "Bar", str
  end

  description "send []= case"
  def test_send_nth_return_val
    str = "bar"
    assert_equal "B", str.[]=(0, "B")
    assert_equal "Bar", str
  end

  description "境界値チェックを詳細にかけておく"
  def test_boundary_value
    s1 = "0123456"

    assert_equal "012", s1[0,3]
    assert_equal "123", s1[1,3]
    assert_equal "", s1[1,0]
    assert_equal "123456", s1[1,30]

    assert_equal nil, s1[3,-1]
    assert_equal nil, s1[3,-3]
    assert_equal nil, s1[3,-4]
    assert_equal nil, s1[3,-5]

    assert_equal "6", s1[6,1]
    assert_equal "", s1[6,0]
    assert_equal "", s1[7,1]     # ??
    assert_equal "", s1[7,0]     # ??
    assert_equal nil, s1[7,-1]
    assert_equal nil, s1[7,-2]
    assert_equal nil, s1[8,1]
    assert_equal nil, s1[8,0]
    assert_equal nil, s1[8,-1]
    assert_equal nil, s1[8,-2]


    assert_equal "", s1[-3,0]
    assert_equal "45", s1[-3,2]
    assert_equal "456", s1[-3,3]
    assert_equal "456", s1[-3,4]

    assert_equal "", s1[-1,0]
    assert_equal "6", s1[-1,1]
    assert_equal "6", s1[-1,2]
    assert_equal "12", s1[-6,2]
    assert_equal "01", s1[-7,2]
    assert_equal nil, s1[-8,0]
    assert_equal nil, s1[-8,1]

    assert_equal nil, s1[-3,-1]
  end

  description "self[nth] = val"
  def test_nth_replace
    s1 = "0123456789"
    s1[0] = "ab"
    assert_equal "ab123456789", s1

    s1 = "0123456789"
    s1[1] = "ab"
    assert_equal "0ab23456789", s1

    s1 = "0123456789"
    s1[9] = "ab"
    assert_equal "012345678ab", s1

    s1 = "0123456789"
    s1[10] = "ab"
    assert_equal "0123456789ab", s1

    s1 = "0123456789"
    s1[-1] = "ab"
    assert_equal "012345678ab", s1

    s1 = "0123456789"
    s1[-2] = "ab"
    assert_equal "01234567ab9", s1

    s1 = "0123456789"
    s1[-10] = "ab"
    assert_equal "ab123456789", s1

    s1 = "0123456789"
    s1[0] = ""
    assert_equal "123456789", s1

    s1 = "0123456789"
    s1[1] = ""
    assert_equal "023456789", s1

    s1 = "0123456789"
    s1[9] = ""
    assert_equal "012345678", s1

    s1 = "0123456789"
    s1[10] = ""
    assert_equal "0123456789", s1

    s1 = "0123456789"
    s1[-1] = ""
    assert_equal "012345678", s1

    s1 = "0123456789"
    s1[-2] = ""
    assert_equal "012345679", s1

    s1 = "0123456789"
    s1[-10] = ""
    assert_equal "123456789", s1
  end

  description "self[nth, len] = val"
  def test_nth_len_replace
    s1 = "0123456789"
    s1[2,5] = "ab"
    assert_equal "01ab789", s1

    s1 = "0123456789"
    s1[2,8] = "ab"
    assert_equal "01ab", s1

    s1 = "0123456789"
    s1[2,9] = "ab"
    assert_equal "01ab", s1

    s1 = "0123456789"
    s1[2,1] = "ab"
    assert_equal "01ab3456789", s1

    s1 = "0123456789"
    s1[2,0] = "ab"
    assert_equal "01ab23456789", s1


    s1 = "0123456789"
    s1[0,5] = "ab"
    assert_equal "ab56789", s1

    s1 = "0123456789"
    s1[0,10] = "ab"
    assert_equal "ab", s1

    s1 = "0123456789"
    s1[0,99] = "ab"
    assert_equal "ab", s1


    s1 = "0123456789"
    s1[9,1] = "ab"
    assert_equal "012345678ab", s1

    s1 = "0123456789"
    s1[10,1] = "ab"
    assert_equal "0123456789ab", s1

    s1 = "0123456789"
    s1[10,0] = "ab"
    assert_equal "0123456789ab", s1


    s1 = "0123456789"
    s1[2,5] = ""
    assert_equal "01789", s1

    s1 = "0123456789"
    s1[2,8] = ""
    assert_equal "01", s1

    s1 = "0123456789"
    s1[2,9] = ""
    assert_equal "01", s1

    s1 = "0123456789"
    s1[2,1] = ""
    assert_equal "013456789", s1

    s1 = "0123456789"
    s1[2,0] = ""
    assert_equal "0123456789", s1


    s1 = "0123456789"
    s1[0,5] = ""
    assert_equal "56789", s1

    s1 = "0123456789"
    s1[0,10] = ""
    assert_equal "", s1

    s1 = "0123456789"
    s1[0,99] = ""
    assert_equal "", s1


    s1 = "0123456789"
    s1[9,1] = ""
    assert_equal "012345678", s1

    s1 = "0123456789"
    s1[10,1] = ""
    assert_equal "0123456789", s1

    s1 = "0123456789"
    s1[10,0] = ""
    assert_equal "0123456789", s1
  end

  description "minus"
  def test_minus
    s1 = "0123456789"
    s1[-8,5] = "ab"
    assert_equal "01ab789", s1

    s1 = "0123456789"
    s1[-8,8] = "ab"
    assert_equal "01ab", s1

    s1 = "0123456789"
    s1[-8,9] = "ab"
    assert_equal "01ab", s1

    s1 = "0123456789"
    s1[-8,1] = "ab"
    assert_equal "01ab3456789", s1

    s1 = "0123456789"
    s1[-8,0] = "ab"
    assert_equal "01ab23456789", s1


    s1 = "0123456789"
    s1[-1,1] = "ab"
    assert_equal "012345678ab", s1

    s1 = "0123456789"
    s1[-1,0] = "ab"
    assert_equal "012345678ab9", s1
  end

  description "self[Range] -> String"
  def test_self_range
    # rangeオブジェクトが終端を含む場合
    assert_equal 'abcd'[ 2 ..  1], ""
    assert_equal 'abcd'[ 2 ..  2], "c"
    assert_equal 'abcd'[ 2 ..  3], "cd"
    assert_equal 'abcd'[ 2 ..  4], "cd"

    assert_equal 'abcd'[ 2 .. -1], "cd"   # str[f..-1] は「f 文字目から
    assert_equal 'abcd'[ 3 .. -1], "d"    # 文字列の最後まで」を表す慣用句

    assert_equal 'abcd'[ 1 ..  2], "bc"
    assert_equal 'abcd'[ 2 ..  2], "c"
    assert_equal 'abcd'[ 3 ..  2], ""
    assert_equal 'abcd'[ 4 ..  2], ""
    assert_equal 'abcd'[ 5 ..  2], nil

    assert_equal 'abcd'[-3 ..  2], "bc"
    assert_equal 'abcd'[-4 ..  2], "abc"
    assert_equal 'abcd'[-5 ..  2], nil

    # rangeオブジェクトが終端を含まない場合
    assert_equal 'abcd'[ 2 ... 3], "c"
    assert_equal 'abcd'[ 2 ... 4], "cd"
    assert_equal 'abcd'[ 2 ... 5], "cd"

    assert_equal 'abcd'[ 1 ... 2], "b"
    assert_equal 'abcd'[ 2 ... 2], ""
    assert_equal 'abcd'[ 3 ... 2], ""
    assert_equal 'abcd'[ 4 ... 2], ""
    assert_equal 'abcd'[ 5 ... 2], nil

    assert_equal 'abcd'[-3 ... 2], "b"
    assert_equal 'abcd'[-4 ... 2], "ab"
    assert_equal 'abcd'[-5 ... 2], nil

    assert_equal 'abcd'[ 1 ..   ], "bcd"
    assert_equal 'abcd'[   .. 2 ], "abc"
  end

  description "self[Range]="
  def test_self_range_replace
    s = 'abcd'; s[ 2 ..  1] = "X"
    assert_equal "abXcd", s
    s = 'abcd'; s[ 2 ..  2] = "X"
    assert_equal "abXd", s
    s = 'abcd'; s[ 2 ..  3] = "X"
    assert_equal "abX", s
    s = 'abcd'; s[ 2 ..  4] = "X"
    assert_equal "abX", s

    s = 'abcd'; s[ 2 .. -1] = "X"
    assert_equal "abX", s
    s = 'abcd'; s[ 3 .. -1] = "X"
    assert_equal "abcX", s

    s = 'abcd'; s[ 1 ..  2] = "X"
    assert_equal "aXd", s
    s = 'abcd'; s[ 2 ..  2] = "X"
    assert_equal "abXd", s
    s = 'abcd'; s[ 3 ..  2] = "X"
    assert_equal "abcXd", s
    s = 'abcd'; s[ 4 ..  2] = "X"
    assert_equal "abcdX", s
    s = 'abcd'
    assert_raise(RangeError) { s[ 5 ..  2] = "X" }

    s = 'abcd'; s[-3 ..  2] = "X"
    assert_equal "aXd", s
    s = 'abcd'; s[-4 ..  2] = "X"
    assert_equal "Xd", s
    s = 'abcd'
    assert_raise(RangeError) { s[-5 ..  2] = "X" }

    s = 'abcd'; s[ 2 ... 3] = "X"
    assert_equal "abXd", s
    s = 'abcd'; s[ 2 ... 4] = "X"
    assert_equal "abX", s
    s = 'abcd'; s[ 2 ... 5] = "X"
    assert_equal "abX", s

    s = 'abcd'; s[ 1 ... 2] = "X"
    assert_equal "aXcd", s
    s = 'abcd'; s[ 2 ... 2] = "X"
    assert_equal "abXcd", s
    s = 'abcd'; s[ 3 ... 2] = "X"
    assert_equal "abcXd", s
    s = 'abcd'; s[ 4 ... 2] = "X"
    assert_equal "abcdX", s
    s = 'abcd'
    assert_raise(RangeError) { s[ 5 ... 2] = "X" }

    s = 'abcd'; s[-3 ... 2] = "X"
    assert_equal "aXcd", s
    s = 'abcd'; s[-4 ... 2] = "X"
    assert_equal "Xcd", s
    s = 'abcd'
    assert_raise(RangeError) { s[-5 ... 2] = "X" }
  end

  description "String#slice!"
  def test_slice_self
    s = "bar"
    assert_equal "r", s.slice!(2)
    assert_equal "ba", s

    s = "bar"
    assert_equal "r", s.slice!(-1)
    assert_equal "ba", s

    s = "bar"
    assert_equal nil, s.slice!(3)
    assert_equal "bar", s

    s = "bar"
    assert_equal nil, s.slice!(-4)
    assert_equal "bar", s

    s = "bar"
    assert_equal "r", s.slice!(2, 1)
    assert_equal "ba", s

    s = "bar"
    assert_equal "",  s.slice!(2, 0)
    assert_equal "bar", s

    s = "bar"
    assert_equal "r", s.slice!(2, 100)
    assert_equal "ba", s

    s = "bar"
    assert_equal "r", s.slice!(-1, 1)
    assert_equal "ba", s

    s = "bar"
    assert_equal "r", s.slice!(-1, 2)
    assert_equal "ba", s

    s = "bar"
    assert_equal "", s.slice!(3, 1)
    assert_equal "bar", s

    s = "bar"
    assert_equal nil, s.slice!(4, 1)
    assert_equal "bar", s

    s = "bar"
    assert_equal nil, s.slice!(-4, 1)
    assert_equal "bar", s
  end

  description "ord"
  def test_ord
    assert_equal 97, "a".ord
    assert_equal 97, "abcde".ord

    assert_raise(ArgumentError, "empty string") do
      "".ord
    end
  end

  description "binary string"
  def test_binary
    s1 = "ABC\x00\x0d\x0e\x0f"
    assert_equal 7, s1.size

    i = 0
    while i < 3
      s1[i] = i.chr
      i += 1
    end
    assert_equal "\x00\x01\x02\x00\x0d\x0e\x0f", s1
    while i < 10
      s1[i] = i.chr
      i += 1
    end
    assert_equal 10, s1.size
    assert_equal "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09", s1
  end

  description "String#size"
  def test_string_size
    s = "abc"
    assert_equal 3, s.size
    assert_equal 3, s.length

    s = ""
    assert_equal 0, s.size
    assert_equal 0, s.length

    s = "\0"
    assert_equal 1, s.size
    assert_equal 1, s.length
  end

  description "index"
  def test_index
    assert_equal 0, "abcde".index("")
    assert_equal 0, "abcde".index("a")
    assert_equal 0, "abcde".index("abc")
    assert_equal 1, "abcde".index("bcd")
    assert_equal 3, "abcde".index("de")
    assert_equal nil, "abcde".index("def")

    assert_equal 2, "abcde".index("c",1)
    assert_equal 2, "abcde".index("c",2)
    assert_equal nil, "abcde".index("c",3)
  end

  description "tr"
  def test_tr
    assert_equal "123defg", "abcdefg".tr("abc", "123")
    assert_equal "123d456", "abcdefg".tr("abcefg", "123456")
    assert_equal "123333g", "abcdefg".tr("abcdef", "123")

    assert_equal "123defg", "abcdefg".tr("a-c", "123")
    assert_equal "123d456", "abcdefg".tr("a-ce-g", "123456")
    assert_equal "123d456", "abcdefg".tr("a-cefg", "123456")

    assert_equal "143defg", "abcdefg".tr("a-cb", "123456")
    assert_equal "143de56", "abcdefg".tr("a-cbfg", "123456")
    assert_equal "14567fg", "abcdefg".tr("a-cb-e", "123456789")

    assert_equal "a999999", "abcdefg".tr("^a", "123456789")
    assert_equal "abc9999", "abcdefg".tr("^abc", "123456789")
    assert_equal "abc9999", "abcdefg".tr("^a-c", "123456789")
    assert_equal "^23defg", "abcdefg".tr("abc", "^23456789")

    # Illegal Cases
    assert_equal "abcdefg",  "abcdefg".tr("", "123456789")
    assert_equal "abc1defg", "abc^defg".tr("^", "123456789")
    assert_equal "abc1defg", "abc-defg".tr("-", "123456789")
    assert_equal "1bc2defg", "abc-defg".tr("a-", "123456789")
    assert_equal "123-defg", "abc-defg".tr("a-c", "123456789")
    assert_equal "ab21defg", "abc-defg".tr("-c", "123456789")

    # delete
    assert_equal "defg", "abcdefg".tr("abc", "")
    assert_equal "abc", "abcdefg".tr("^abc", "")

    # range replace
    assert_equal "FOO", "foo".tr('a-z', 'A-Z')
    assert_equal "foo", "FOO".tr('A-Z', 'a-z')
    assert_equal "12cABC", "abcdef".tr("abd-f", "12A-C")
  end

  description "start_with?"
  def test_start_with
    assert_true  "abc".start_with?("")
    assert_true  "abc".start_with?("a")
    assert_true  "abc".start_with?("ab")
    assert_true  "abc".start_with?("abc")
    assert_false "abc".start_with?("abcd")
    assert_false "abc".start_with?("A")
    assert_false "abc".start_with?("aA")
    assert_false "abc".start_with?("b")
  end

  description "end_with?"
  def test_end_with
    assert_true  "abc".end_with?("")
    assert_true  "abc".end_with?("c")
    assert_true  "abc".end_with?("bc")
    assert_true  "abc".end_with?("abc")
    assert_false "abc".end_with?(" abc")
    assert_false "abc".end_with?("C")
    assert_false "abc".end_with?("Bc")
    assert_false "abc".end_with?("b")
  end

  description "include?"
  def test_include
    assert_true  "abc".include?("")
    assert_true  "abc".include?("a")
    assert_true  "abc".include?("b")
    assert_true  "abc".include?("c")
    assert_true  "abc".include?("ab")
    assert_true  "abc".include?("bc")
    assert_true  "abc".include?("abc")
    assert_false "abc".include?("abcd")
    assert_false "abc".include?(" abc")
    assert_false "abc".include?("A")
    assert_false "abc".include?("aB")
    assert_false "abc".include?("abC")
  end

  description "to_f, to_i, to_s"
  def test_to_something
    assert_equal 10.0, "10".to_f
    assert_equal 1000.0, "10e2".to_f
    assert_equal 0.25, "25e-2".to_f
    assert_equal 0.25, ".25".to_f

    # not support this case.
    #assert_equal 0.0, "nan".to_f
    #assert_equal 0.0, "INF".to_f
    #assert_equal( -0.0, "-Inf".to_f )

    assert_equal 0.0, "".to_f
    #assert_equal 100.0, "1_0_0".to_f
    assert_equal 10.0, " \n10".to_f
    #assert_equal 0.0, "0xa.a".to_f

    assert_equal 10, " 10".to_i
    assert_equal 10, "+10".to_i
    assert_equal( -10, "-10".to_i )

    assert_equal 10, "010".to_i
    assert_equal( -10, "-010".to_i )

    assert_equal 0, "0x11".to_i
    assert_equal 0, "".to_i

    assert_equal 1, "01".to_i(2)
    #assert_equal 1, "0b1".to_i(2)

    assert_equal 7, "07".to_i(8)
    #assert_equal 7, "0o7".to_i(8)

    assert_equal 31, "1f".to_i(16)
    #assert_equal 31, "0x1f".to_i(16)

    # not support this case.
    #assert_equal 2, "0b10".to_i(0)
    #assert_equal 8, "0o10".to_i(0)
    #assert_equal 8, "010".to_i(0)
    #assert_equal 10, "0d10".to_i(0)
    #assert_equal 16, "0x10".to_i(0)

    assert_equal "str", "str".to_s
  end

  description "String#bytes chars"
  def test_string_bytes_chars
    assert_equal [97, 98, 99], "abc".bytes
  end

  description "String#bytes empty"
  def test_string_bytes_empty
    assert_equal [], "".bytes
  end

  description "String#bytes null char"
  def test_string_bytes_null_char
    assert_equal [97, 0, 98], "a\000b".bytes
  end

  description "String#dup"
  def test_string_dup
    assert_equal "a", "a".dup
    assert_equal "abc", "abc".dup
    assert_equal "", "".dup
  end

  description "String#empty?"
  def test_string_empty_question
    assert_true  "".empty?
    assert_false "a".empty?
    assert_false "abc".empty?
    assert_false " ".empty?
    assert_false "\0".empty?
  end

  description "String#clear"
  def test_string_clear
    s = "abc"
    assert_equal "", s.clear
    assert_equal "", s
  end

  description "String#getbyte"
  def test_string_getbyte
    s = "abc"
    assert_equal 97, s.getbyte(0)
    assert_equal 98, s.getbyte(1)
    assert_equal 99, s.getbyte(2)
    assert_equal nil, s.getbyte(3)
    assert_equal 99, s.getbyte(-1)
  end

  description "String#byteslice"
  def test_string_byteslice
    s = "hello"
    assert_equal "h", s.byteslice(0)
    assert_equal "o", s.byteslice(-1)
    assert_equal nil, s.byteslice(5)
    assert_equal nil, s.byteslice(-6)
    assert_equal "el", s.byteslice(1, 2)
    assert_equal "", s.byteslice(1, 0)
    assert_equal nil, s.byteslice(1, -1)
    assert_equal "ell", s.byteslice(1..3)
    assert_equal "el", s.byteslice(1...3)
  end

  description "String#setbyte"
  def test_string_setbyte
    s = "ABCDE"
    assert_equal 0x61, s.setbyte(0, 0x61)
    assert_equal "aBCDE", s
    assert_equal 0x65, s.setbyte(4, 0x65)
    assert_equal "aBCDe", s
    assert_raise(IndexError) { s.setbyte(5, 0) }

    s = "ABCDE"
    assert_equal 0x65, s.setbyte(-1, 0x65)
    assert_equal "ABCDe", s
    assert_equal 0x61, s.setbyte(-5, 0x61)
    assert_equal "aBCDe", s
    assert_raise(IndexError) { s.setbyte(-6, 0) }
  end

  description "String#inspect"
  def test_string_inspect
    assert_equal "\"\\x00\"", "\0".inspect
    assert_equal "\"foo\"", "foo".inspect
  end

  description "String#upcase"
  def string_upcase
    assert_equal "ABC", "abc".upcase
    assert_equal "ABC", "ABC".upcase
    assert_equal "ABC", "aBc".upcase
    assert_equal "ABC", "AbC".upcase
    assert_equal "ABC", "aBC".upcase
    assert_equal "ABC", "Abc".upcase
    assert_equal "ABC", "abc".upcase
    assert_equal "A\0BC", "a\0bc".upcase
  end

  description "String#upcase!"
  def test_string_upcase_bang
    str = "\0abc"
    ret = str.upcase!
    assert_equal "\0ABC", ret
    assert_equal "\0ABC", str
    assert_equal str, ret
    assert_nil "ABC".upcase!
  end

  description "String#downcase"
  def test_string_downcase
    assert_equal "abc", "abc".downcase
    assert_equal "a\0bc", "A\0BC".downcase
    assert_equal "abc", "aBc".downcase
    assert_equal "abc", "AbC".downcase
    assert_equal "abc", "aBC".downcase
    assert_equal "abc", "Abc".downcase
    assert_equal "abc", "abc".downcase
  end

  description "String#downcase!"
  def test_string_downcase_bang
    str = "A\0BC"
    ret = str.downcase!
    assert_equal "a\0bc", ret
    assert_equal "a\0bc", str
    assert_equal str, ret
    assert_nil "abc".downcase!
  end

  description "String#each_line"
  def test_string_each_line
    str = "line1\nline2\r\nline3\rline4"
    lines = []
    str.each_line { |line| lines << line }
    assert_equal ["line1\n", "line2\r\n", "line3\rline4"], lines
    lines.clear
    str.each_line("\r") { |line| lines << line }
    assert_equal ["line1\nline2\r", "\nline3\r", "line4"], lines
    lines.clear
    str.each_line(chomp: true) { |line| lines << line }
    assert_equal ["line1", "line2", "line3\rline4"], lines
  end

end

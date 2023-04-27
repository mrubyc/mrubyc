# frozen_string_literal: true

class StringSplitTest < MrubycTestCase

  #
  # Regex not supported.
  #

  description "String"
  def string_case
    assert_equal ["a","b","c"],             "a,b,c".split(",")
    assert_equal ["a","","b","c"],          "a,,b,c".split(",")
    assert_equal ["a","b:c","d"],           "a::b:c::d".split("::")
    assert_equal ["a","b:c","d:"],          "a::b:c::d:".split("::")
    assert_equal ["a","b:c","d"],           "a::b:c::d::".split("::")
    assert_equal ["a", "b:c", "d", ":"],    "a::b:c::d:::".split("::")
    # assert_equal ["あ", "い", "う"],        "あ,い,う".split(",")
    # assert_equal ["あ", "", "い", "う"],    "あ,,い,う".split(",")
    # assert_equal ["あ", "い", "う"],        "あ、い、う".split("、")
    # assert_equal ["ああ", "い", "う"],        "ああ、い、う".split("、")
    # assert_equal ["ああ", "い", "う"],        "ああ、、い、、う".split("、、")
  end

  description "space"
  def space_case
    assert_equal ["a", "b", "c"], "   a \t  b \n  c\r\n".split(" ")
    assert_equal ["a", "b", "c"], "a \t  b \n  c\r\n".split(" ")
    assert_equal ["a", "b", "c"], "   a \t  b \n  c".split(" ")
    assert_equal ["a", "b", "c"], "a \t  b \n  c".split(" ")
    assert_equal ["aa", "bb", "cc"], " aa bb cc ".split(" ")
    assert_equal ["aa", "bb", "cc"], "aa bb cc ".split(" ")
    assert_equal ["aa", "bb", "cc"], " aa bb cc".split(" ")
    assert_equal ["aa", "bb", "cc"], "aa bb cc".split(" ")
  end

  description "nil"
  def nil_case
    assert_equal ["a", "b", "c"], "   a \t  b \n  c".split()
    assert_equal ["a", "b", "c"], "   a \t  b \n  c".split(nil)
  end

  description "empty string"
  def empty_string_case
    assert_equal [" ", " ", " ", "a", " ", "\t", " ", " ", "b", " ", "\n", " ", " ", "c"], "   a \t  b \n  c".split("")
  end

  description "limit"
  def limit_case
    assert_equal ["a", "b", "", "c"],         "a,b,,c,,".split(",", 0)
    assert_equal ["a,b,,c,,"],                "a,b,,c,,".split(",", 1)
    assert_equal ["a", "b,,c,,"],             "a,b,,c,,".split(",", 2)
    assert_equal ["a", "b", ",c,,"],          "a,b,,c,,".split(",", 3)
    assert_equal ["a", "b", "", "c,,"],       "a,b,,c,,".split(",", 4)
    assert_equal ["a", "b", "", "c", ","],    "a,b,,c,,".split(",", 5)
    assert_equal ["a", "b", "", "c", "", ""], "a,b,,c,,".split(",", 6)
    assert_equal ["a", "b", "", "c", "", ""], "a,b,,c,,".split(",", 7)
    assert_equal ["a", "b", "", "c", "", ""], "a,b,,c,,".split(",", -1)
    assert_equal ["a", "b", "", "c", "", ""], "a,b,,c,,".split(",", -2)

    assert_equal ["aa", "bb", "cc"],        " aa  bb  cc ".split(" ", 0)
    assert_equal [" aa  bb  cc "],          " aa  bb  cc ".split(" ", 1)
    assert_equal ["aa", "bb  cc "],         " aa  bb  cc ".split(" ", 2)
    assert_equal ["aa", "bb", "cc "],       " aa  bb  cc ".split(" ", 3)
    assert_equal ["aa", "bb", "cc", ""],    " aa  bb  cc ".split(" ", 4)
    assert_equal ["aa", "bb", "cc", ""],    " aa  bb  cc ".split(" ", 5)
    assert_equal ["aa", "bb", "cc", ""],    " aa  bb  cc ".split(" ",-1)

    assert_equal ["aa", "bb", "cc"],        "aa  bb  cc".split(" ", 0)
    assert_equal ["aa  bb  cc"],            "aa  bb  cc".split(" ", 1)
    assert_equal ["aa", "bb  cc"],          "aa  bb  cc".split(" ", 2)
    assert_equal ["aa", "bb", "cc"],        "aa  bb  cc".split(" ", 3)
    assert_equal ["aa", "bb", "cc"],        "aa  bb  cc".split(" ", 4)
    assert_equal ["aa", "bb", "cc"],        "aa  bb  cc".split(" ",-1)
  end

  description "empty source"
  def empty_source_case
    assert_equal [], "".split(",")
    assert_equal [], "".split(",", 0)
    assert_equal [], "".split(",", 1)
    assert_equal [], "".split(",",-1)

    assert_equal [], "".split("")
    assert_equal [], "".split("", 0)
    assert_equal [], "".split("", 1)
    assert_equal [], "".split("",-1)

    assert_equal [], "".split(" ")
    assert_equal [], "".split(" ", 0)
    assert_equal [], "".split(" ", 1)
    assert_equal [], "".split(" ",-1)
  end

  description "delimiter only"
  def delimiter_only_case
    assert_equal [],        ",".split(",")
    assert_equal [],        ",".split(",", 0)
    assert_equal [","],     ",".split(",", 1)
    assert_equal ["",""],   ",".split(",",-1)

    assert_equal [],        ",,".split(",")
    assert_equal [],        ",,".split(",", 0)
    assert_equal [",,"],    ",,".split(",", 1)
    assert_equal ["","",""],",,".split(",",-1)

    assert_equal [],        " ".split(" ")
    assert_equal [],        " ".split(" ", 0)
    assert_equal [" "],     " ".split(" ", 1)
    assert_equal [""],      " ".split(" ",-1)

    assert_equal [],        "  ".split(" ")
    assert_equal [],        "  ".split(" ", 0)
    assert_equal ["  "],    "  ".split(" ", 1)
    assert_equal [""],      "  ".split(" ",-1)
  end

end

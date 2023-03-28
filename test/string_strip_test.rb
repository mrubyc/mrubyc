
class StringStripTest < MrubycTestCase

  description "strip, lstrip, rstrip"
  def strip_case
    s = " a b c "
    assert_equal "a b c", s.strip
    assert_equal "a b c ", s.lstrip
    assert_equal " a b c", s.rstrip

    s = "abc"
    assert_equal "abc", s.strip
    assert_equal "abc", s.lstrip
    assert_equal "abc", s.rstrip

    s = " abc"
    assert_equal "abc", s.strip
    assert_equal "abc", s.lstrip
    assert_equal " abc", s.rstrip

    s = "abc "
    assert_equal "abc", s.strip
    assert_equal "abc ", s.lstrip
    assert_equal "abc", s.rstrip

    s = " "
    assert_equal "", s.strip
    assert_equal "", s.lstrip
    assert_equal "", s.rstrip

    s = ""
    assert_equal "", s.strip
    assert_equal "", s.lstrip
    assert_equal "", s.rstrip

    s = " \t\r\n\f\vABC \t\r\n\f\v\0"
    assert_equal "ABC", s.strip
    assert_equal "ABC \t\r\n\f\v\0", s.lstrip
    assert_equal " \t\r\n\f\vABC", s.rstrip

    s = " \0ABC\0 "
    assert_equal "\0ABC", s.strip
    assert_equal "\0ABC\0 ", s.lstrip
    assert_equal " \0ABC", s.rstrip

    s = "\0"
    assert_equal "", s.strip
    assert_equal "\0", s.lstrip
    assert_equal "", s.rstrip

    s = " あ い う "
    assert_equal "あ い う", s.strip
    assert_equal "あ い う ", s.lstrip
    assert_equal " あ い う", s.rstrip

    s = "あいう"
    assert_equal "あいう", s.strip
    assert_equal "あいう", s.lstrip
    assert_equal "あいう", s.rstrip

    s = " あいう"
    assert_equal "あいう", s.strip
    assert_equal "あいう", s.lstrip
    assert_equal " あいう", s.rstrip

    s = "あいう "
    assert_equal "あいう", s.strip
    assert_equal "あいう ", s.lstrip
    assert_equal "あいう", s.rstrip

    s = " \t\r\n\f\vあいう \t\r\n\f\v\0"
    assert_equal "あいう", s.strip
    assert_equal "あいう \t\r\n\f\v\0", s.lstrip
    assert_equal " \t\r\n\f\vあいう", s.rstrip

    s = " \0あいう\0 "
    assert_equal "\0あいう", s.strip
    assert_equal "\0あいう\0 ", s.lstrip
    assert_equal " \0あいう", s.rstrip
  end

  description "strip!"
  def strip_bang_case
    s = " a b c "
    assert_equal "a b c", s.strip!
    assert_equal "a b c", s

    s = "abc"
    assert_equal nil, s.strip!
    assert_equal "abc", s

    s = " abc"
    assert_equal "abc", s.strip!
    assert_equal "abc", s

    s = "abc "
    assert_equal "abc", s.strip!
    assert_equal "abc", s

    s = " "
    assert_equal "", s.strip!
    assert_equal "", s

    s = ""
    assert_equal nil, s.strip!
    assert_equal "", s

    s = " \t\r\n\f\vABC \t\r\n\f\v\0"
    assert_equal "ABC", s.strip!
    assert_equal "ABC", s

    s = " \0ABC\0 "
    assert_equal "\0ABC", s.strip!
    assert_equal "\0ABC", s

    s = "\0"
    assert_equal "", s.strip!
    assert_equal "", s

    s = " あ い う "
    assert_equal "あ い う", s.strip!

    # FIXME
    # s = "あいう"
    # assert_equal "あいう", s.strip!

    s = " あいう"
    assert_equal "あいう", s.strip!

    s = "あいう "
    assert_equal "あいう", s.strip!

    s = " \t\r\n\f\vあいう \t\r\n\f\v\0"
    assert_equal "あいう", s.strip!

    s = " \0あいう\0 "
    assert_equal "\0あいう", s.strip!
  end

  description "lstrip!"
  def lstrip_bang_case
    s = " a b c "
    assert_equal "a b c ", s.lstrip!
    assert_equal "a b c ", s

    s = "abc"
    assert_equal nil, s.lstrip!
    assert_equal "abc", s

    s = " abc"
    assert_equal "abc", s.lstrip!
    assert_equal "abc", s

    s = "abc "
    assert_equal nil, s.lstrip!
    assert_equal "abc ", s

    s = " "
    assert_equal "", s.lstrip!
    assert_equal "", s

    s = ""
    assert_equal nil, s.lstrip!
    assert_equal "", s

    s = " \t\r\n\f\vABC \t\r\n\f\v\0"
    assert_equal "ABC \t\r\n\f\v\0", s.lstrip!
    assert_equal "ABC \t\r\n\f\v\0", s

    s = " \0ABC\0 "
    assert_equal "\0ABC\0 ", s.lstrip!
    assert_equal "\0ABC\0 ", s

    s = "\0"
    assert_equal nil, s.lstrip!
    assert_equal "\0", s

    s = " あ い う "
    assert_equal "あ い う ", s.lstrip!

    # FIXME
    # s = "あいう"
    # assert_equal "あいう", s.lstrip!

    s = " あいう"
    assert_equal "あいう", s.lstrip!

    # FIXME
    # s = "あいう "
    # assert_equal "あいう ", s.lstrip!

    s = " \t\r\n\f\vあいう \t\r\n\f\v\0"
    assert_equal "あいう \t\r\n\f\v\0", s.lstrip!

    s = " \0あいう\0 "
    assert_equal "\0あいう\0 ", s.lstrip!
  end

  description "rstrip!"
  def rstrip_bang_case
    s = " a b c "
    assert_equal " a b c", s.rstrip!
    assert_equal " a b c", s

    s = "abc"
    assert_equal nil, s.rstrip!
    assert_equal "abc", s

    s = " abc"
    assert_equal nil, s.rstrip!
    assert_equal " abc", s

    s = "abc "
    assert_equal "abc", s.rstrip!
    assert_equal "abc", s

    s = " "
    assert_equal "", s.rstrip!
    assert_equal "", s

    s = ""
    assert_equal nil, s.rstrip!
    assert_equal "", s

    s = " \t\r\n\f\vABC \t\r\n\f\v\0"
    assert_equal " \t\r\n\f\vABC", s.rstrip!
    assert_equal " \t\r\n\f\vABC", s

    s = " \0ABC\0 "
    assert_equal " \0ABC", s.rstrip!
    assert_equal " \0ABC", s

    s = "\0"
    assert_equal "", s.rstrip!
    assert_equal "", s

    s = " あ い う "
    assert_equal " あ い う", s.rstrip!

    # FIXME
    # s = "あいう"
    # assert_equal "あいう", s.rstrip!

    # FIXME
    # s = " あいう"
    # assert_equal " あいう", s.rstrip!

    s = "あいう "
    assert_equal "あいう", s.rstrip!

    s = " \t\r\n\f\vあいう \t\r\n\f\v\0"
    assert_equal " \t\r\n\f\vあいう", s.rstrip!

    s = " \0あいう\0 "
    assert_equal " \0あいう", s.rstrip!
  end

  description "chomp"
  def chomp_case
    s1 = "foo\r\n"
    s2 = s1.chomp
    assert_equal "foo\r\n", s1
    assert_equal "foo", s2

    s1 = "foo\r"
    s2 = s1.chomp
    assert_equal "foo\r", s1
    assert_equal "foo", s2

    s1 = "foo\n"
    s2 = s1.chomp
    assert_equal "foo\n", s1
    assert_equal "foo", s2

    s1 = "foo"
    s2 = s1.chomp
    assert_equal "foo", s1
    assert_equal "foo", s2

    s1 = ""
    s2 = s1.chomp
    assert_equal "", s1
    assert_equal "", s2

    s1 = "\r\n"
    s2 = s1.chomp
    assert_equal "\r\n", s1
    assert_equal "", s2

    s1 = "foo\r\n\r\n"
    s2 = s1.chomp
    assert_equal "foo\r\n\r\n", s1
    assert_equal "foo\r\n", s2

    s1 = "あいう\r\n"
    s2 = s1.chomp
    assert_equal "あいう\r\n", s1
    assert_equal "あいう", s2

    s1 = "あいう\r"
    s2 = s1.chomp
    assert_equal "あいう\r", s1
    assert_equal "あいう", s2

    s1 = "あいう\n"
    s2 = s1.chomp
    assert_equal "あいう\n", s1
    assert_equal "あいう", s2

    s1 = "あいう"
    s2 = s1.chomp
    assert_equal "あいう", s1
    assert_equal "あいう", s2

    s1 = "あいう\r\n\r\n"
    s2 = s1.chomp
    assert_equal "あいう\r\n\r\n", s1
    assert_equal "あいう\r\n", s2

    s1 = "あいう\r\nあいう\r\n"
    s2 = s1.chomp
    assert_equal "あいう\r\nあいう\r\n", s1
    assert_equal "あいう\r\nあいう", s2
  end

  description "chomp!"
  def chomp_bang_case
    s = "foo\r\n"
    assert_equal "foo", s.chomp!
    assert_equal "foo", s

    s = "foo"
    assert_equal nil, s.chomp!
    assert_equal "foo", s

    s = ""
    assert_equal nil, s.chomp!
    assert_equal "", s

    s = "\r\n"
    assert_equal "", s.chomp!
    assert_equal "", s

    s = "あいう\r\n"
    assert_equal "あいう", s.chomp!
    assert_equal "あいう", s

    s = "あいう"
    assert_equal nil, s.chomp!
    assert_equal "あいう", s

    s = "あいう\r\nあいう\r\n"
    s = s.chomp!
    assert_equal "あいう\r\nあいう", s
  end

end

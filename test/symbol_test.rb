
class SymbolTest < Picotest::Test

  description "生成"
  def test_identity
    s = :symbol
    assert_equal :symbol, s
  end

  description "比較"
  def test_eq
    s = :symbol
    assert_equal true, s == :symbol
    assert_equal false, s == :symbol2
    assert_equal false, s != :symbol
    assert_equal true, s != :symbol2
  end

  description "<=> comparison"
  def test_compare
    assert (:a <=> :b) < 0
    assert (:c <=> :b) > 0
    assert (:string <=> :stringAA) < 0
    assert (:string <=> :string) == 0
    assert (:stringAA <=> :string) > 0
  end

  description "to_sym"
  def test_to_sym
    s = "abc"
    assert_equal :abc, s.to_sym
    assert_not_equal :abc, s
  end

  description "to_s"
  def test_to_s
    s = :symbol
    assert_equal "symbol", s.to_s
    assert_not_equal "symbol", s
  end

  description "inspect"
  def test_inspect
    assert_equal ':":a"', :":a".inspect
  end
end

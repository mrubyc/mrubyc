
class PatternMatchTest < Picotest::Test

  description "value pattern with integer"
  def test_value_pattern_with_integer
    result = case 1
    in 1
      :match
    end
    assert_equal :match, result
  end

  description "value pattern with string"
  def test_value_pattern_with_string
    result = case "hello"
    in "hello"
      :match
    end
    assert_equal :match, result
  end

  description "value pattern with symbol"
  def test_value_pattern_with_symbol
    result = case :foo
    in :foo
      :match
    end
    assert_equal :match, result
  end

  description "value pattern with true"
  def test_value_pattern_with_true
    result = case true
    in true
      :match
    end
    assert_equal :match, result
  end

  description "value pattern with false"
  def test_value_pattern_with_false
    result = case false
    in false
      :match
    end
    assert_equal :match, result
  end

  description "value pattern with nil"
  def test_value_pattern_with_nil
    result = case nil
    in nil
      :match
    end
    assert_equal :match, result
  end

  description "variable pattern"
  def test_variable_pattern
    result = case 42
    in x
      x
    end
    assert_equal 42, result
  end

  description "multiple in clauses"
  def test_multiple_in_clauses
    result = case 2
    in 1
      :one
    in 2
      :two
    in 3
      :three
    end
    assert_equal :two, result
  end

  description "else clause"
  def test_else_clause
    result = case 99
    in 1
      :one
    in 2
      :two
    else
      :other
    end
    assert_equal :other, result
  end

  description "no match raises"
  def test_no_match_raises
    assert_raise(NoMatchingPatternError) do
      case 99
      in 1
        :one
      in 2
        :two
      end
    end
  end

  description "alternative pattern"
  def test_alternative_pattern
    result = case 2
    in 1 | 2 | 3
      :match
    end
    assert_equal :match, result
  end

  description "array pattern simple"
  def test_array_pattern_simple
    result = case [1, 2, 3]
    in [1, 2, 3]
      :match
    end
    assert_equal :match, result
  end

  description "array pattern with variables"
  def test_array_pattern_with_variables
    result = case [1, 2, 3]
    in [a, b, c]
      [a, b, c]
    end
    assert_equal [1, 2, 3], result
  end

  description "array pattern exact length"
  def test_array_pattern_exact_length
    result = case [1, 2]
    in [1, 2, 3]
      :no_match
    in [1, 2]
      :match
    end
    assert_equal :match, result
  end

  description "array pattern 2 elements"
  def test_array_pattern_2_elements
    result = case [1, 2]
    in [a, b]
      [a, b]
    end
    assert_equal [1, 2], result
  end

  description "array pattern with wildcard"
  def test_array_pattern_with_wildcard
    result = case [1, 2, 3]
    in [_, x, _]
      x
    end
    assert_equal 2, result
  end

  description "guard clause if"
  def test_guard_clause_if
    result = case 10
    in x if 5 < x
      :big
    in x
      :small
    end
    assert_equal :big, result
  end

  description "guard clause unless"
  def test_guard_clause_unless
    result = case 3
    in x unless 5 < x
      :small
    in x
      :big
    end
    assert_equal :small, result
  end

  description "guard clause with array pattern"
  def test_guard_clause_with_array_pattern
    result = case [1, 2, 3]
    in [a, b, c] if 5 < a + b + c
      :sum_big
    in [a, b, c]
      :sum_small
    end
    assert_equal :sum_big, result
  end

  description "guard clause failing"
  def test_guard_clause_failing
    result = case 10
    in x if 20 < x
      :first
    in x
      :second
    end
    assert_equal :second, result
  end

  description "pin operator basic"
  def test_pin_operator_basic
    x = 1
    result = case 1
    in ^x
      :matched
    else
      :not_matched
    end
    assert_equal :matched, result
  end

  description "pin operator no match"
  def test_pin_operator_no_match
    a = 1
    result = case 2
    in ^a
      :same
    else
      :different
    end
    assert_equal :different, result
  end

  description "pin operator in array"
  def test_pin_operator_in_array
    expected = 42
    result = case [42, 100]
    in [^expected, y]
      y
    end
    assert_equal 100, result
  end

  description "alternative in array pattern"
  def test_alternative_in_array_pattern
    result = case [3, 4]
    in [1, 2] | [3, 4]
      :match
    else
      :no_match
    end
    assert_equal :match, result
  end

  description "array pattern with literal and variable"
  def test_array_pattern_with_literal_and_variable
    result = case [1, 2, 3]
    in [1, 2, x]
      x
    end
    assert_equal 3, result
  end

  description "empty array pattern"
  def test_empty_array_pattern
    result = case []
    in []
      :empty
    else
      :not_empty
    end
    assert_equal :empty, result
  end

  description "alternative pattern first branch"
  def test_alternative_pattern_first_branch
    result = case 1
    in 1 | 2 | 3
      :found
    else
      :not_found
    end
    assert_equal :found, result
  end

  description "alternative pattern third branch"
  def test_alternative_pattern_third_branch
    result = case 3
    in 1 | 2 | 3
      :found
    else
      :not_found
    end
    assert_equal :found, result
  end

  description "alternative pattern no match"
  def test_alternative_pattern_no_match
    result = case 5
    in 1 | 2 | 3
      :found
    else
      :not_found
    end
    assert_equal :not_found, result
  end

  class Foo
    def deconstruct
      nil
    end
  end
  class Bar
    def deconstruct_keys(keys)
      nil
    end
  end
  class Baz
    def deconstruct
      nil
    end
  end

  description "deconstruct returns nil"
  def test_deconstruct_returns_nil
    result = case Foo.new
    in [a, b]
      :match
    else
      :no_match
    end
    assert_equal :no_match, result
  end

  description "deconstruct_keys returns nil"
  def test_deconstruct_keys_returns_nil
    result = case Bar.new
    in {a: 1}
      :match
    else
      :no_match
    end
    assert_equal :no_match, result
  end

  description "deconstruct returns nil for find pattern"
  def test_deconstruct_returns_nil_for_find_pattern
    result = case Baz.new
    in [*, 1, 2, *]
      :match
    else
      :no_match
    end
    assert_equal :no_match, result
  end

  description "hash pattern with nil matches exact keys"
  def test_hash_pattern_with_nil_matches_exact_keys
    result = case {a: 1}
    in {a: 1, **nil}
      :match
    else
      :no_match
    end
    assert_equal :match, result
  end

  description "hash pattern with nil rejects extra keys"
  def test_hash_pattern_with_nil_rejects_extra_keys
    result = case {a: 1, b: 2}
    in {a: 1, **nil}
      :match
    else
      :no_match
    end
    assert_equal :no_match, result
  end

  description "hash pattern with missing key does not match"
  def test_hash_pattern_with_missing_key_does_not_match
    result = case {b: 1}
    in {a: nil}
      :match
    else
      :no_match
    end
    assert_equal :no_match, result
  end

  description "hash pattern with nil value matches"
  def test_hash_pattern_with_nil_value_matches
    result = case {a: nil}
    in {a: nil}
      :match
    else
      :no_match
    end
    assert_equal :match, result
  end

  description "case with splat in array literal predicate"
  def test_case_with_splat_in_array_literal_predicate
    arr = [2, 3]
    result = case [1, *arr, 4]
    in [1, 2, 3, 4]
      :match
    else
      :no_match
    end
    assert_equal :match, result
  end

  description "array pattern with rest"
  def test_array_pattern_with_rest
    result = case [1, 2, 3, 4, 5]
    in [first, *rest, last]
      [first, rest, last]
    end
    assert_equal [1, [2, 3, 4], 5], result
  end

  description "array pattern nested simple"
  def test_array_pattern_nested_simple
    result = case [[1, 2]]
    in [[a, b]]
      [a, b]
    end
    assert_equal [1, 2], result
  end

  description "array pattern nested"
  def test_array_pattern_nested
    result = case [[1, 2], [3, 4]]
    in [[a, b], [c, d]]
      [a, b, c, d]
    end
    assert_equal [1, 2, 3, 4], result
  end

  description "hash pattern simple"
  def test_hash_pattern_simple
    result = case {a: 1, b: 2}
    in {a: 1, b: 2}
      :match
    end
    assert_equal :match, result
  end

  description "hash pattern with variables"
  def test_hash_pattern_with_variables
    result = case {a: 1, b: 2}
    in {a: x, b: y}
      [x, y]
    end
    assert_equal [1, 2], result
  end

  description "hash pattern shorthand"
  def test_hash_pattern_shorthand
    result = case {a: 1, b: 2}
    in {a:, b:}
      [a, b]
    end
    assert_equal [1, 2], result
  end

  description "hash pattern partial match"
  def test_hash_pattern_partial_match
    result = case {a: 1, b: 2, c: 3}
    in {a: x}
      x
    end
    assert_equal 1, result
  end

  description "hash pattern nested"
  def test_hash_pattern_nested
    result = case {a: {x: 1}, b: {y: 2}}
    in {a: {x: n}, b: {y: m}}
      [n, m]
    end
    assert_equal [1, 2], result
  end

  description "find pattern basic"
  def test_find_pattern_basic
    result = case [1, 2, 3, 4, 5]
    in [*, 2, 3, *]
      :match
    end
    assert_equal :match, result
  end

  description "find pattern with capture"
  def test_find_pattern_with_capture
    result = case [1, 2, 3, 4, 5]
    in [*pre, 2, 3, *post]
      [pre, post]
    end
    assert_equal [[1], [4, 5]], result
  end

  description "find pattern no match"
  def test_find_pattern_no_match
    assert_raise(NoMatchingPatternError) do
      case [1, 2, 3]
      in [*, 5, 6, *]
        :match
      end
    end
  end

  description "find pattern at beginning"
  def test_find_pattern_at_beginning
    result = case [1, 2, 3]
    in [*pre, 1, *post]
      [pre, post]
    end
    assert_equal [[], [2, 3]], result
  end

  description "find pattern at end"
  def test_find_pattern_at_end
    result = case [1, 2, 3]
    in [*pre, 3, *post]
      [pre, post]
    end
    assert_equal [[1, 2], []], result
  end

  description "find pattern multiple elements"
  def test_find_pattern_multiple_elements
    result = case [1, 2, 3, 4, 5]
    in [*pre, 2, x, *post]
      [pre[0], x, post]
    end
    assert_equal [1, 3, [4, 5]], result
  end

  description "as pattern with array"
  def test_as_pattern_with_array
    result = case [1, 2, 3]
    in [x, *rest] => whole
      [x, rest, whole]
    end
    assert_equal [1, [2, 3], [1, 2, 3]], result
  end

  description "as pattern with hash"
  def test_as_pattern_with_hash
    result = case {a: 1, b: 2}
    in {a:} => h
      [a, h]
    end
    assert_equal [1, {a: 1, b: 2}], result
  end

  description "array pattern with first element"
  def test_array_pattern_with_first_element
    result = case [1, 2, 3, 4, 5]
    in [first, *rest]
      [first, rest]
    end
    assert_equal [1, [2, 3, 4, 5]], result
  end

  description "deeply nested array and hash"
  def test_deeply_nested_array_and_hash
    result = case {outer: [{inner: [1, 2]}, 3]}
    in {outer: [{inner: [a, b]}, c]}
      [a, b, c]
    end
    assert_equal [1, 2, 3], result
  end

  description "complex pattern with all features"
  def test_complex_pattern_with_all_features
    result = case [1, 2, 3, 4, 5]
    in [1, *rest] => arr if 2 < arr.size
      [rest[0], rest[1..1], rest[2..-1]]
    end
    assert_equal [2, [3], [4, 5]], result
  end

end

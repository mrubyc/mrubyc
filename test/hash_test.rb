
class HashTest < Picotest::Test

  description "生成"
  def test_constructor
    h = {a: 1, b: 2}
    assert_equal( {:a=>1, :b=>2 }, h )

    h = Hash.new
    assert_equal( {}, h )
  end

  description "値の取り出し"
  def test_pick_value
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( "value", h[:key] )
    assert_equal( :value,  h["key"] )
    assert_equal( "Three", h[3] )
    assert_equal( 444,     h[:"4"] )
  end

  description "値の設定、サイズ"
  def test_setter
    h = Hash.new
    assert_equal( 0, h.size )
    h["key"] = :value
    assert_equal( :value,  h["key"] )
    assert_equal( 1, h.size )
  end

  description "存在しないキーにアクセスした場合nilが返ること"
  def test_nonexistent
    h = Hash.new
    assert_equal( nil, h["no-exist-key"] )
  end

  description "operator !="
  def test_negate_op
    h = Hash.new
    h1 = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_not_equal( h, h1 )
  end

  description "==  (順序が違うHash同士の比較)"
  def test_eq_eq
    h1 = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    h = Hash.new
    h["key"] = :value
    h[:key] = "value"
    h[:"4"] = 444
    h[3] = "Three"
    assert_equal( h, h1 )

    assert_equal( 4, h.size )
    assert_equal( 4, h1.size )
  end

  description "値の上書き"
  def test_override
    h = Hash.new
    h["key"] = :value
    h[:"4"] = 444
    h[3] = "Three"
    h1 = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    h[:key] = "other value"
    assert_not_equal( h, h1 )
    assert_equal( 4, h.size )
  end

  description "clear"
  def test_clear
    h = {}
    h[:key] = "other value"
    h.clear
    assert_equal( {}, h )
    assert_equal( 0, h.size )
  end

  description "dup"
  def test_dup
    h = {:a=>"A", :b=>"B"}
    h1 = h
    assert_equal( h, h1 )

    h1[:a] = "AA"
    assert_equal( h, h1 )

    h1 = h.dup
    h1[:b] = "BB"
    assert_not_equal( h, h1 )
  end

  description "delete"
  def test_delete
    h = {:ab => "some", :cd => "all"}

    assert_equal( "some", h.delete(:ab) )
    assert_equal( h, {:cd=>"all"} )

    assert_equal( nil, h.delete(:ef) )

    # TODO delete with block
  end

  description "empty?"
  def test_is_empty
    assert_equal( true, {}.empty? )
    assert_equal( false, {:a=>1}.empty? )
  end

  description "has_key?"
  def test_has_key
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( true,  h.has_key?(:key) )
    assert_equal( false, h.has_key?(:key2) )
  end

  description "has_value?"
  def test_has_value
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( true,  h.has_value?(444) )
    assert_equal( false, h.has_value?(555) )
  end

  description "key"
  def test_key
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( "key", h.key(:value) )
    assert_equal( nil,   h.key(:no_exist) )
  end

  description "keys"
  def test_keys
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( [:key, "key", 3, :"4"], h.keys )
  end

  description "values"
  def test_values
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( ["value", :value, "Three", 444], h.values )
  end

  description "size, length, count"
  def test_size
    h = {:key=>"value", "key"=>:value, 3=>"Three", :"4"=>444}
    assert_equal( 4, h.size )
    assert_equal( 4, h.length )
    assert_equal( 4, h.count )
  end

  description "merge"
  def test_merge
    foo = {1=>"a", 2=>"b", 3=>"c"}
    bar = {2=>"B", 3=>"C", 4=>"D"}
    assert_equal( {1=>"a", 2=>"B", 3=>"C", 4=>"D"}, foo.merge(bar) )
    assert_equal( {1=>"a", 2=>"b", 3=>"c"}, foo )
    assert_equal( {2=>"B", 3=>"C", 4=>"D"}, bar )
  end

  description "merge!"
  def test_merge_bung
    foo = {1=>"a", 2=>"b", 3=>"c"}
    bar = {2=>"B", 3=>"C", 4=>"D"}
    assert_equal( {1=>"a", 2=>"B", 3=>"C", 4=>"D"}, foo.merge!(bar) )
    assert_equal( {1=>"a", 2=>"B", 3=>"C", 4=>"D"}, foo )
    assert_equal( {2=>"B", 3=>"C", 4=>"D"}, bar )
  end

  description "to_h"
  def test_to_h
    h = {}
    assert_equal( {}, h.to_h )
  end

  description "Hash#deconstruct_keys with nil"
  def test_deconstruct_keys_nil
    h = {a: 1, b: 2, c: 3}
    result = h.deconstruct_keys(nil)
    assert_equal({a: 1, b: 2, c: 3}, result)
    assert_equal h, result
  end

  description "Hash#deconstruct_keys with array of keys"
  def test_deconstruct_keys_array
    h = {a: 1, b: 2, c: 3}
    # In simple implementation, returns self regardless of keys
    result = h.deconstruct_keys([:a, :b])
    assert_equal({a: 1, b: 2, c: 3}, result)
  end

  description "Hash#deconstruct_keys with empty array"
  def test_deconstruct_keys_empty_array
    h = {a: 1, b: 2}
    result = h.deconstruct_keys([])
    assert_equal({a: 1, b: 2}, result)
  end

  description "Hash#deconstruct_keys with empty hash"
  def test_deconstruct_keys_empty_hash
    h = {}
    result = h.deconstruct_keys(nil)
    assert_equal({}, result)
  end

  description "Hash#deconstruct_keys with nested hash"
  def test_deconstruct_keys_nested
    h = {outer: {inner: 1}, b: 2}
    result = h.deconstruct_keys(nil)
    assert_equal({outer: {inner: 1}, b: 2}, result)
  end

  description "Hash#deconstruct_keys raises ArgumentError with no arguments"
  def test_deconstruct_keys_no_args
    h = {a: 1}
    assert_raise(ArgumentError) { h.deconstruct_keys }
  end

  description "Hash#deconstruct_keys raises ArgumentError with too many arguments"
  def test_deconstruct_keys_too_many_args
    h = {a: 1}
    assert_raise(ArgumentError) { h.deconstruct_keys(nil, nil) }
  end

end

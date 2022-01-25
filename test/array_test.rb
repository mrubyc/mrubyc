# frozen_string_literal: true

class ArrayTest < MrubycTestCase

  description "operator +"
  def operator_case
    assert_equal [1,2,3,4], [1,2] + [3,4]
    a = [1,2,3]
    b = a + [4,5]
    assert_equal [1,2,3], a
    assert_equal [1,2,3,4,5], b
    a += b
    assert_equal [1,2,3,1,2,3,4,5], a
    assert_equal [1,2,3,4,5], b
  end

  description "size, length, empty, clear"
  def size_case
    a = [0,1,2,3,4]
    e = []
    assert_equal 5, a.size
    assert_equal 5, a.length
    assert_equal 5, a.count
    assert_equal 0, e.size

    assert_equal a, a
    assert_equal [0,1,2,3,4], a
    assert_equal e, e
    assert_equal [], e

    assert_equal false, a != a
    assert_equal true, a != e

    assert_equal true, e.empty?
    assert_equal false, a.empty?

    assert_equal [], a.clear
    assert_equal [], a
    assert_equal true, a.empty?
  end

  description "constructor"
  def constructor_case
    assert_equal [], Array.new
    assert_equal [nil, nil, nil, nil, nil], Array.new(5)
    assert_equal [0, 0, 0, 0, 0], Array.new(5,0)
    assert_equal ["AB","CD","E"], %w(AB CD E)
    assert_equal [:AB, :CD, :E], %i(AB CD E)
  end

  description "setter"
  def setter_case
    a = Array.new
    assert_equal 0, a[0] = 0
    assert_equal [0], a
    a[1] = 1
    assert_equal [0,1], a
    a[3] = 3
    assert_equal [0,1,nil,3], a
    a[0] = 99
    assert_equal [99,1,nil,3], a
    a[1] = 88
    assert_equal [99,88,nil,3], a

    a[-1] = 77
    assert_equal [99,88,nil,77], a
    a[-2] = 66
    assert_equal [99,88,66,77], a

    assert_equal 44, a[6,50] = 44
    assert_equal [99,88,66,77,nil,nil,44], a
    a[2,1] = 77  
    assert_equal [99,88,77,77,nil,nil,44], a
    a[3,0] = 66
    assert_equal [99,88,77,66,77,nil,nil,44], a
    a[4,3] = 55
    assert_equal [99,88,77,66,55,44], a
    a[4,10] = 44
    assert_equal [99,88,77,66,44], a
  end

  description "getter"
  def getter_case
    a = [1,2,3,4]
    assert_equal 1, a[0]
    assert_equal 1, a.at(0)
    assert_equal 2, a[1]
    assert_equal 3, a[2]
    assert_equal 4, a[3]
    assert_equal nil, a[4]

    assert_equal 4, a[-1]
    assert_equal 3, a[-2]
    assert_equal 2, a[-3]
    assert_equal 1, a[-4]
    assert_equal nil, a[-5]

    assert_equal [2,3], a[1,2]
    assert_equal [2,3,4], a[1,10000]
    assert_equal [4], a[3,1]
    assert_equal [4], a[3,2]
    assert_equal nil, a[10000,2]
    assert_equal nil, a[10000,10000]

    assert_equal [2,3], a[-3,2]
    assert_equal [2,3,4], a[-3,10000]
    assert_equal [1], a[-4,1]
    assert_equal nil, a[-5,1]
    assert_equal nil, a[-10000,1]
    assert_equal nil, a[-10000,10000]
    assert_equal nil, a[-10000,-10000]
  end

  description "index / first / last"
  def index_case
    a = [1,2,3,4]
    e = []
    assert_equal 1, a.index(2)
    assert_equal nil, a.index(0)
    assert_equal nil, e.index(9)

    assert_equal 1, a.first
    assert_equal 4, a.last
    assert_equal nil, e.first
    assert_equal nil, e.last
  end

  description "delete_at"
  def delete_at_case
    a = [1,2,3,4]
    assert_equal 1, a.delete_at(0)
    assert_equal [2,3,4], a
    assert_equal 3, a.delete_at(1)
    assert_equal [2,4], a
    assert_equal 4, a.delete_at(-1)
    assert_equal [2], a
  end

  description "push / pop"
  def push_case
    a = []
    assert_equal [1], a.push(1)
    assert_equal [1,2], a.push(2)
    assert_equal [1,2], a

    assert_equal 2, a.pop()
    assert_equal [1], a
    assert_equal 1, a.pop()
    assert_equal [], a
    assert_equal nil, a.pop()
    assert_equal [], a

    a = [1,2,3,4]
    assert_equal [2,3,4], a.pop(3)
    assert_equal [1], a
    assert_equal [1], a.pop(2)
    assert_equal [], a

    a = []
    assert_equal [1], a << 1
    assert_equal [1,2], a << 2
    assert_equal [1,2], a
  end

  description "unshift / shift"
  def shift_case
    a = []
    assert_equal [1], a.unshift(1)
    assert_equal [2,1], a.unshift(2)
    assert_equal [2,1], a

    assert_equal 2, a.shift()
    assert_equal [1], a
    assert_equal 1, a.shift()
    assert_equal [], a
    assert_equal nil, a.shift()
    assert_equal [], a

    a = [1,2,3,4]
    assert_equal [1,2,3], a.shift(3)
    assert_equal [4], a
    assert_equal [4], a.shift(2)
    assert_equal [], a
  end

  description "dup"
  def dup_case
    a = [1,2,3]
    b = a
    a[0] = 11
    assert_equal a, b
    assert_equal [11,2,3], b

    a = [1,2,3]
    b = a.dup
    a[0] = 11
    assert_not_equal a, b
    assert_equal [11,2,3], a
    assert_equal [1,2,3], b
  end

  description "min, max, minmax"
  def minmax_case
    a = %w(albatross dog horse)
    assert_equal "albatross", a.min
    assert_equal "horse", a.max
    assert_equal ["albatross","horse"], a.minmax

    a = ["AAA"]
    assert_equal "AAA", a.min
    assert_equal "AAA", a.max
    assert_equal ["AAA","AAA"], a.minmax

    a = []
    assert_equal nil, a.min
    assert_equal nil, a.max
    assert_equal [nil,nil], a.minmax
  end

  description "inspect, to_s, join"
  def inspect_case
    a = [1,2,3]
    assert_equal "[1, 2, 3]",       a.inspect
    assert_equal "[1, 2, 3]",       a.to_s
    assert_equal "123",             a.join
    assert_equal "1,2,3",           a.join(",")
    assert_equal "1, 2, 3",         a.join(", ")

    array = [1,"AA",:sym]
    hash = {1=>1, :k2=>:v2, "k3"=>"v3"}
    range = 1..3
    a = [nil, false, true, 123, 2.718, :symbol, array, "string", range, hash]
    assert_equal %q![nil, false, true, 123, 2.718, :symbol, [1, "AA", :sym], "string", 1..3, {1=>1, :k2=>:v2, "k3"=>"v3"}]!, a.inspect
    assert_equal %q![nil, false, true, 123, 2.718, :symbol, [1, "AA", :sym], "string", 1..3, {1=>1, :k2=>:v2, "k3"=>"v3"}]!, a.to_s
    assert_equal %q!,false,true,123,2.718,symbol,1,AA,sym,string,1..3,{1=>1, :k2=>:v2, "k3"=>"v3"}!, a.join(",")
  end

  description "each"
  def each_case

    a = [1,2,3]
    $cnt = 0
    a.each {|a1|
      $cnt += 1
      assert_equal a1, $cnt
    }
    assert_equal 3, $cnt

    $cnt = 0
    a.each_index {|i|
      assert_equal i, $cnt
      $cnt += 1
      assert_equal a[i], $cnt
    }
    assert_equal 3, $cnt

    $cnt = 0
    a.each_with_index {|a1,i|
      $cnt += 1
      assert_equal a1, $cnt
      assert_equal a[i], $cnt
    }
    assert_equal 3, $cnt
  end

  description "collect"
  def collect_case
    a = [1,2,3]
    assert_equal [2,4,6], a.collect {|a1| a1 * 2}
    assert_equal [1,2,3], a

    a.collect! {|a1| a1 * 2}
    assert_equal [2,4,6], a
  end

end

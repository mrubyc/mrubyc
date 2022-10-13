#
# Array, mrubyc class library
#
#  Copyright (C) 2015-2020 Kyushu Institute of Technology.
#  Copyright (C) 2015-2020 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

class Array

  #
  # collect
  #
  def collect
    i = 0
    ary = []
    while i < length
      ary[i] = yield self[i]
      i += 1
    end
    return ary
  end
  alias map collect

  #
  # collect!
  #
  def collect!
    i = 0
    while i < length
      self[i] = yield self[i]
      i += 1
    end
    return self
  end
  alias map! collect!

  #
  # delete_if
  #
  def delete_if
    i = 0
    while i < length
      if yield self[i]
        delete_at(i)
      else
        i += 1
      end
    end
    return self
  end

  #
  # each
  #
  def each
    i = 0
    while i < length
      yield self[i]
      i += 1
    end
    return self
  end

  #
  # each index
  #
  def each_index
    i = 0
    while i < length
      yield i
      i += 1
    end
    return self
  end

  #
  # each with index
  #
  def each_with_index
    i = 0
    while i < length
      yield self[i], i
      i += 1
    end
    return self
  end

  #
  # index
  #
  def index( *val, &block )
    i = 0

    # case of index {|item| ...} -> Integer | nil
    if block
      while i < length
        return i  if yield self[i]
        i += 1
      end
      return nil
    end

    # case of index(val) -> Integer | nil
    if !val.empty?
      while i < length
        return i  if self[i] == val[0]
        i += 1
      end
      return nil
    end

    raise ArgumentError
  end

  #
  # reject!
  #
  def reject!( &block )
    n = length
    delete_if( &block )
    return n == length ? nil : self
  end

  #
  # reject
  #
  def reject( &block )
    return self.dup.delete_if( &block )
  end

  #
  # sort!
  #
  def sort!( &block )
    n = length - 1
    i = 0
    while i < n
      j = i
      while j < n
        j += 1
        v_i = self[i]
        v_j = self[j]
        if block
          next if block.call(v_i, v_j) <= 0
        else
          next if v_i <= v_j
        end
        self[i] = v_j
        self[j] = v_i
      end
      i += 1
    end
    return self
  end

  #
  # sort
  #
  def sort( &block )
    return self.dup.sort!( &block )
  end

end

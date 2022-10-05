#
# String, mrubyc class library
#
#  Copyright (C) 2015-2018 Kyushu Institute of Technology.
#  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

class String

  ##
  # Passes each byte in str to the given block.
  #
  def each_byte
    idx = 0
    while idx < length
      yield self.getbyte(idx)
      idx += 1
    end
    self
  end

  ##
  # Passes each character in str to the given block.
  #
  def each_char
    idx = 0
    while idx < length
      yield self[idx]
      idx += 1
    end
    self
  end

  #
  # ljust
  #
  def ljust(width, padding = ' ')
    __ljust_rjust_argcheck(width, padding)
    result = self.dup
    return result if width <= length
    while true
      padding.each_char do |char|
        result << char
        return result if result.length == width
      end
    end
  end

  #
  # rjust
  #
  def rjust(width, padding = ' ')
    __ljust_rjust_argcheck(width, padding)
    return self.dup if width <= length
    rlen = width - length
    rstr = ""
    while true
      rstr << padding
      break if rlen <= rstr.length
    end
    rstr[0, rlen] + self
  end

  # private

    def __ljust_rjust_argcheck(width, padding)
      if padding.length == 0
        raise ArgumentError, "zero width padding"
      end
      unless width.kind_of?(Integer)
        raise TypeError, "no implicit conversion into Integer"
      end
    end

end

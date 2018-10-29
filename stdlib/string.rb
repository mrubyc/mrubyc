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
      yield self[idx].ord
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

end

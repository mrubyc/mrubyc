#
# Hash, mrubyc class library
#
#  Copyright (C) 2015-      Kyushu Institute of Technology.
#  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
#  Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#

class Hash
  include Enumerable

  #
  # each
  #
  def each(&block)
    keys = self.keys
    len = length
    i = 0
    while i < len
      key = keys[i]
      block.call([key, self[key]])
      i += 1
    end

    return self
  end
end

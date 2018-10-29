#
# Range, mrubyc class library
#
#  Copyright (C) 2015-2018 Kyushu Institute of Technology.
#  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

class Range

  # each
  def each
    val = self.first
    last = self.last

    lim = last
    lim += 1 unless exclude_end?
    i = val
    while i < lim do
      yield i
      i += 1
    end
    return self
  end

end

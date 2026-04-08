#
# Enumerable module for mruby/c.
#
#  Copyright (C) 2015-      Kyushu Institute of Technology.
#  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
#  Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#

module Enumerable
  #
  # collect, map
  #
  def collect
    ret = []
    self.each {|item|
      ret << yield( item )
    }
    return ret
  end
  alias map collect

  #
  # each with index
  #
  def each_with_index
    i = 0
    self.each {|item|
      yield( item, i )
      i += 1
    }
    return self
  end

end

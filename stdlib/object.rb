#
# Object, mrubyc class library
#
#  Copyright (C) 2015-2018 Kyushu Institute of Technology.
#  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

class Object
  RUBY_VERSION = "1.9"
  MRUBYC_VERSION = "1.1"

  # loop
  def loop
    while true 
      yield
    end
  end


end

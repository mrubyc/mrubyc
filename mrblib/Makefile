#
# mruby/c  Makefile
#
# Copyright (C) 2015-2019 Kyushu Institute of Technology.
# Copyright (C) 2015-2019 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

SRC = array.rb global.rb hash.rb numeric.rb object.rb range.rb string.rb
OUTPUT = ../src/mrblib.c
MRBC ?= mrbc


all: $(OUTPUT)

$(OUTPUT): $(SRC)
	cat $(SRC) > mrblib.rb
	$(MRBC) -Bmrblib_bytecode --remove-lv -o$(OUTPUT) mrblib.rb
	rm -f mrblib.rb

clean:
	@rm -f mrblib.rb *~

distclean: clean
	@rm -f $(OUTPUT)

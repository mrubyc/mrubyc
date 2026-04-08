#
# mruby/c  Makefile
#
# Copyright (C) 2015-      Kyushu Institute of Technology.
# Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
# Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#

TARGET = ../src/mrblib.c
VERSIONFILE =  ../src/VERSION
SRC = enum.rb array.rb global.rb hash.rb numeric.rb object.rb range.rb string.rb
MRBC ?= mrbc


all: $(TARGET) $(VERSIONFILE)

$(TARGET): $(SRC)
	$(MRBC) -Bmrblib_bytecode --remove-lv -o$(TARGET) $(SRC)

$(VERSIONFILE): global.rb
	sed -E -e '/^[A-Z]+_VERSION/!d' $? > $@

.PHONY: clean distclean clean_all
clean:
	@rm -f *~

distclean:
	rm -f $(TARGET) $(VERSIONFILE) *~

clean_all: distclean

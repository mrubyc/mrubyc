#
# mruby/c mrblib/Makefile
# Compile Ruby files into RITE bytecode and outputs them as a C-lang array.
#
# Copyright (C) 2015-      Kyushu Institute of Technology.
# Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
# Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#

TARGETS = $(MRBLIB_C) $(VERSIONFILE)
MRBLIB_C = ../src/mrblib.c
VERSIONFILE =  ../src/VERSION
SRCS = enum.rb array.rb global.rb hash.rb numeric.rb object.rb range.rb string.rb
MRBC ?= mrbc


#
# Targets
#
.PHONY: default all clean clean_all autogen

## Default target
default: $(TARGETS)

## (re)Build all targets
all: clean_all
	@$(MAKE) $(TARGETS)

## Remove editor backup files
clean:
	rm -f *~

## Remove intermediate files
clean_all: clean
	rm -f $(TARGETS)

## dummy
autogen:


#
# How to build each file
#
$(MRBLIB_C): $(SRCS)
	$(MRBC) -Bmrblib_bytecode --remove-lv -o$@ $(SRCS)

$(VERSIONFILE): global.rb
	sed -E -e '/^[A-Z]+_VERSION/!d' $? > $@

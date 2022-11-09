#
# mruby/c  src/Makefile
#
# Copyright (C) 2015-2022 Kyushu Institute of Technology.
# Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

include hal_selector.mk

TARGET = libmrubyc.a
CFLAGS += -Wall -Wpointer-arith -g  # -std=c99 -pedantic -pedantic-errors
SRCS = $(HAL_DIR)/hal.c alloc.c c_array.c c_hash.c c_math.c c_numeric.c \
	c_object.c c_range.c c_string.c class.c console.c error.c global.c \
	keyvalue.c load.c mrblib.c rrt0.c symbol.c value.c vm.c
OBJS = $(SRCS:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	@rm -f $(TARGET) $(OBJS) *~

clean_all:	clean
	@rm -f $(AUTOGEN_SYMBOL_TABLE) $(AUTOGEN_METHOD_TABLE)

autogen:
	@rm -f $(AUTOGEN_SYMBOL_TABLE) $(AUTOGEN_METHOD_TABLE)
	$(MAKE) $(AUTOGEN_SYMBOL_TABLE)

check_depend:
	$(CC) $(CFLAGS) -MM $(SRCS) | sed 's/hal_[^ ]*\/hal\./$$(HAL_DIR)\/hal./g'


# Auto generated files.

MAKE_SYMBOL_TABLE ?= ../support/make_symbol_table.rb
MAKE_METHOD_TABLE ?= ../support/make_method_table.rb

AUTOGEN_SYMBOL_TABLE = _autogen_builtin_symbol.h
AUTOGEN_METHOD_TABLE = _autogen_class_array.h _autogen_class_exception.h \
	_autogen_class_float.h _autogen_class_hash.h _autogen_class_integer.h \
	_autogen_class_math.h _autogen_class_object.h _autogen_class_range.h \
	_autogen_class_string.h _autogen_class_symbol.h

#
# un-comment bellow, if you need add and/or delete method in builtin class.
#
#AUTOGEN_METHOD_SRCS = c_array.c c_hash.c c_math.c c_numeric.c c_object.c c_range.c c_string.c error.c

$(AUTOGEN_SYMBOL_TABLE): $(AUTOGEN_METHOD_TABLE)
	$(MAKE_SYMBOL_TABLE) --path-c . --path-rb ../mrblib -o $(AUTOGEN_SYMBOL_TABLE)

_autogen_class_array.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_array.c
_autogen_class_integer.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_numeric.c
_autogen_class_float.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_numeric.c
_autogen_class_hash.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_hash.c
_autogen_class_math.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_math.c
_autogen_class_object.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_object.c
_autogen_class_range.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_range.c
_autogen_class_string.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_string.c
_autogen_class_symbol.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) symbol.c
_autogen_class_exception.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) error.c


# File dependencies.

hal.o: $(HAL_DIR)/hal.c $(HAL_DIR)/hal.h

alloc.o: alloc.c vm_config.h alloc.h hal_selector.h $(HAL_DIR)/hal.h \
  console.h value.h
c_array.o: c_array.c vm_config.h alloc.h value.h class.h keyvalue.h \
  error.h c_string.h c_array.h console.h _autogen_class_array.h \
  _autogen_builtin_symbol.h
c_hash.o: c_hash.c vm_config.h alloc.h value.h class.h keyvalue.h error.h \
  c_string.h c_array.h c_hash.h _autogen_class_hash.h \
  _autogen_builtin_symbol.h
c_math.o: c_math.c vm_config.h value.h symbol.h _autogen_builtin_symbol.h \
  class.h keyvalue.h error.h global.h
c_numeric.o: c_numeric.c vm_config.h value.h class.h keyvalue.h error.h \
  c_string.h console.h _autogen_class_integer.h \
  _autogen_builtin_symbol.h _autogen_class_float.h
c_object.o: c_object.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h error.h class.h keyvalue.h c_string.h \
  c_array.h c_hash.h vm.h console.h _autogen_class_object.h
c_range.o: c_range.c vm_config.h alloc.h value.h class.h keyvalue.h \
  error.h c_string.h c_range.h console.h _autogen_class_range.h \
  _autogen_builtin_symbol.h
c_string.o: c_string.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h class.h keyvalue.h error.h c_string.h \
  c_array.h vm.h console.h _autogen_class_string.h
class.o: class.c vm_config.h alloc.h value.h vm.h class.h keyvalue.h \
  error.h symbol.h _autogen_builtin_symbol.h global.h console.h load.h \
  c_string.h c_array.h c_hash.h
console.o: console.c vm_config.h hal_selector.h $(HAL_DIR)/hal.h value.h \
  class.h keyvalue.h error.h console.h symbol.h \
  _autogen_builtin_symbol.h c_string.h c_array.h alloc.h c_hash.h \
  c_range.h
error.o: error.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h error.h class.h keyvalue.h c_string.h vm.h \
  _autogen_class_exception.h
global.o: global.c vm_config.h value.h global.h keyvalue.h class.h \
  error.h symbol.h _autogen_builtin_symbol.h console.h
keyvalue.o: keyvalue.c vm_config.h value.h alloc.h keyvalue.h
load.o: load.c vm_config.h vm.h value.h class.h keyvalue.h error.h load.h \
  alloc.h symbol.h _autogen_builtin_symbol.h c_string.h
mrblib.o: mrblib.c
rrt0.o: rrt0.c vm_config.h alloc.h load.h value.h class.h keyvalue.h \
  error.h global.h symbol.h _autogen_builtin_symbol.h vm.h console.h \
  rrt0.h hal_selector.h $(HAL_DIR)/hal.h
symbol.o: symbol.c vm_config.h _autogen_builtin_symbol.h alloc.h value.h \
  class.h keyvalue.h error.h c_string.h c_array.h console.h \
  _autogen_class_symbol.h
value.o: value.c vm_config.h value.h symbol.h _autogen_builtin_symbol.h \
  class.h keyvalue.h error.h c_string.h c_range.h c_array.h alloc.h \
  c_hash.h
vm.o: vm.c vm_config.h alloc.h value.h symbol.h _autogen_builtin_symbol.h \
  class.h keyvalue.h error.h c_string.h c_range.h c_array.h c_hash.h \
  global.h load.h console.h opcode.h vm.h

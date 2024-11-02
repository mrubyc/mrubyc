#
# mruby/c  src/Makefile
#
# Copyright (C) 2015- Kyushu Institute of Technology.
# Copyright (C) 2015- Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

include hal_selector.mk

TARGET = $(BUILD_DIR)/libmrubyc.a
CFLAGS += -Wall -g   #-std=c99 -pedantic -pedantic-errors
SRCS = alloc.c c_array.c c_hash.c c_math.c c_module.c c_numeric.c \
	c_object.c c_range.c c_string.c class.c console.c error.c global.c \
	keyvalue.c load.c mrblib.c rrt0.c symbol.c value.c vm.c hal.c
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
BUILD_DIR = ../build


.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $?

$(BUILD_DIR)/%.o: %.c
	@-mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean clean_all autogen check_depend
clean:
	@rm -f $(TARGET) $(OBJS) *~

clean_all: clean
	@rm -f $(AUTOGEN_SYMBOL_TABLE) $(AUTOGEN_METHOD_TABLE)

autogen:
	@rm -f $(AUTOGEN_SYMBOL_TABLE) $(AUTOGEN_METHOD_TABLE)
	$(MAKE) $(AUTOGEN_SYMBOL_TABLE)

check_depend:
	@$(CC) $(CFLAGS) -MM `echo $(SRCS) | sed 's/hal.c//'` \
	  | sed -e 's/\(^.*\.o:\)/$$(BUILD_DIR)\/\1/' -e 's/[^ ]*hal.h/$$(HAL_DIR)\/hal.h/'


# Auto generated files.

MAKE_SYMBOL_TABLE ?= ../support/make_symbol_table.rb
MAKE_METHOD_TABLE ?= ../support/make_method_table.rb

AUTOGEN_SYMBOL_TABLE = _autogen_builtin_symbol.h
AUTOGEN_METHOD_TABLE = _autogen_class_array.h _autogen_class_exception.h \
	_autogen_class_float.h _autogen_class_hash.h _autogen_class_integer.h \
	_autogen_module_math.h _autogen_module_module.h _autogen_class_object.h \
	_autogen_class_range.h _autogen_class_string.h _autogen_class_symbol.h \
	_autogen_class_rrt0.h

#
# un-comment below, if you need add and/or delete method in builtin class.
#
#AUTOGEN_METHOD_SRCS = c_array.c c_hash.c c_math.c c_module.c c_numeric.c c_object.c c_range.c c_string.c error.c rrt0.c

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
_autogen_module_math.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_math.c
_autogen_module_module.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_module.c
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
_autogen_class_rrt0.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) rrt0.c


# File dependencies.

$(BUILD_DIR)/hal.o: $(HAL_DIR)/hal.c $(HAL_DIR)/hal.h
	$(CC) -c $(CFLAGS) -o $(BUILD_DIR)/hal.o $<

$(BUILD_DIR)/alloc.o: alloc.c vm_config.h alloc.h $(HAL_DIR)/hal.h console.h value.h
$(BUILD_DIR)/c_array.o: c_array.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h class.h keyvalue.h error.h c_string.h \
  c_array.h console.h _autogen_class_array.h
$(BUILD_DIR)/c_hash.o: c_hash.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h class.h keyvalue.h error.h c_string.h \
  c_array.h c_hash.h _autogen_class_hash.h
$(BUILD_DIR)/c_math.o: c_math.c vm_config.h value.h symbol.h _autogen_builtin_symbol.h \
  error.h class.h keyvalue.h global.h
$(BUILD_DIR)/c_module.o: c_module.c vm_config.h value.h error.h class.h keyvalue.h \
  console.h _autogen_module_module.h _autogen_builtin_symbol.h
$(BUILD_DIR)/c_numeric.o: c_numeric.c vm_config.h value.h symbol.h \
  _autogen_builtin_symbol.h error.h class.h keyvalue.h c_string.h \
  console.h _autogen_class_integer.h _autogen_class_float.h
$(BUILD_DIR)/c_object.o: c_object.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h error.h class.h keyvalue.h c_object.h \
  c_string.h c_array.h c_hash.h global.h vm.h console.h \
  _autogen_class_object.h
$(BUILD_DIR)/c_range.o: c_range.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h class.h keyvalue.h error.h c_string.h \
  c_range.h console.h _autogen_class_range.h
$(BUILD_DIR)/c_string.o: c_string.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h class.h keyvalue.h error.h c_string.h \
  c_array.h vm.h console.h _autogen_class_string.h
$(BUILD_DIR)/class.o: class.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h error.h keyvalue.h class.h c_string.h \
  c_array.h c_hash.h c_math.h global.h vm.h load.h console.h
$(BUILD_DIR)/console.o: console.c vm_config.h $(HAL_DIR)/hal.h alloc.h value.h \
  symbol.h _autogen_builtin_symbol.h class.h keyvalue.h error.h \
  console.h c_string.h c_array.h c_hash.h c_range.h global.h
$(BUILD_DIR)/error.o: error.c vm_config.h alloc.h value.h symbol.h \
  _autogen_builtin_symbol.h error.h class.h keyvalue.h c_string.h vm.h \
  console.h _autogen_class_exception.h
$(BUILD_DIR)/global.o: global.c vm_config.h value.h symbol.h _autogen_builtin_symbol.h \
  global.h keyvalue.h class.h error.h console.h c_array.h alloc.h
$(BUILD_DIR)/keyvalue.o: keyvalue.c vm_config.h value.h alloc.h keyvalue.h
$(BUILD_DIR)/load.o: load.c vm_config.h vm.h value.h class.h keyvalue.h error.h \
  alloc.h symbol.h _autogen_builtin_symbol.h c_string.h load.h
$(BUILD_DIR)/mrblib.o: mrblib.c
$(BUILD_DIR)/rrt0.o: rrt0.c vm_config.h alloc.h load.h value.h class.h keyvalue.h \
  error.h global.h symbol.h _autogen_builtin_symbol.h vm.h console.h \
  c_string.h c_array.h rrt0.h $(HAL_DIR)/hal.h _autogen_class_rrt0.h
$(BUILD_DIR)/symbol.o: symbol.c vm_config.h _autogen_builtin_symbol.h alloc.h value.h \
  symbol.h class.h keyvalue.h error.h c_string.h c_array.h console.h \
  _autogen_class_symbol.h
$(BUILD_DIR)/value.o: value.c vm_config.h value.h symbol.h _autogen_builtin_symbol.h \
  class.h keyvalue.h error.h c_string.h c_range.h c_array.h alloc.h \
  c_hash.h
$(BUILD_DIR)/vm.o: vm.c vm_config.h alloc.h value.h symbol.h _autogen_builtin_symbol.h \
  class.h keyvalue.h error.h c_string.h c_range.h c_array.h c_hash.h \
  global.h load.h console.h opcode.h vm.h

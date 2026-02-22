#
# mruby/c  src/Makefile
#
# Copyright (C) 2015- Kyushu Institute of Technology.
# Copyright (C) 2015- Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

include hal_selector.mk

# Delete the target file if its recipe exits with an error,
# preventing stale or corrupt generated files from being reused.
.DELETE_ON_ERROR:

TARGET = $(BUILD_DIR)/libmrubyc.a
CFLAGS += -Wall -g   #-std=c99 -pedantic -pedantic-errors

# Optional feature flags (pass to make, e.g., make MRBC_USE_STRING_UTF8=1)
ifdef MRBC_USE_STRING_UTF8
CFLAGS += -DMRBC_USE_STRING_UTF8=$(MRBC_USE_STRING_UTF8)
endif
ifdef MRBC_USE_UNICODE_CASE
CFLAGS += -DMRBC_USE_UNICODE_CASE=$(MRBC_USE_UNICODE_CASE)
endif
SRCS = alloc.c c_array.c c_hash.c c_math.c c_numeric.c c_object.c c_proc.c \
       c_range.c c_string.c class.c console.c error.c global.c keyvalue.c \
       load.c mrblib.c rrt0.c symbol.c value.c vm.c hal.c
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
	@rm -rf $(TARGET) $(OBJS) $(BUILD_DIR) *~
	@rm -f $(AUTOGEN_SYMBOL_TABLE) $(AUTOGEN_CLASS_TABLE) $(AUTOGEN_METHOD_TABLE) $(AUTOGEN_UNICODE_CASE)

clean_all: clean

autogen:
	$(MAKE) $(AUTOGEN_SYMBOL_TABLE) $(AUTOGEN_UNICODE_CASE)

check_depend:
	@$(CC) $(CFLAGS) -MM `echo $(SRCS) | sed 's/hal.c//'` \
	  | sed -e 's/\(^.*\.o:\)/$$(BUILD_DIR)\/\1/' -e 's/[^ ]*hal.h/$$(HAL_DIR)\/hal.h/'


#
# Auto generated files.
#
MAKE_SYMBOL_TABLE ?= ../support/make_symbol_table.rb
MAKE_CLASS_TABLE ?= ../support/make_class_table.rb
MAKE_METHOD_TABLE ?= ../support/make_method_table.rb
GENERATE_UNICODE_CASE ?= ../support/generate_unicode_case_tables.rb

# To upgrade Unicode, update UNICODE_VERSION and replace UnicodeData.txt in the
# repository with the new version from:
#   https://www.unicode.org/Public/<version>/ucd/UnicodeData.txt
# See doc/compile.md for the upgrade procedure.
UNICODE_VERSION = 16.0.0
UNICODE_DATA ?= UnicodeData.txt
AUTOGEN_UNICODE_CASE = _autogen_unicode_case.h

AUTOGEN_SYMBOL_TABLE = _autogen_builtin_symbol.h
AUTOGEN_CLASS_TABLE = _autogen_builtin_class.h
AUTOGEN_METHOD_TABLE = _autogen_class_array.h _autogen_class_exception.h \
	_autogen_class_float.h _autogen_class_hash.h _autogen_class_integer.h \
	_autogen_module_math.h _autogen_class_object.h _autogen_class_proc.h \
	_autogen_class_range.h _autogen_class_string.h _autogen_class_symbol.h \
	_autogen_class_rrt0.h

#
# un-comment below, if you need add and/or delete method in builtin class.
#
#AUTOGEN_METHOD_SRCS = c_object.c symbol.c c_numeric.c c_string.c c_proc.c c_range.c c_array.c c_hash.c error.c c_math.c rrt0.c

$(AUTOGEN_UNICODE_CASE): $(UNICODE_DATA)
	ruby $(GENERATE_UNICODE_CASE) $(UNICODE_DATA) $@

$(AUTOGEN_SYMBOL_TABLE): $(AUTOGEN_CLASS_TABLE)
	$(MAKE_SYMBOL_TABLE) --path-c . --path-rb ../mrblib -o $@

$(AUTOGEN_CLASS_TABLE): $(AUTOGEN_METHOD_TABLE)
	$(MAKE_CLASS_TABLE) -o $@ c_object.c symbol.c c_numeric.c c_string.c c_proc.c c_range.c c_array.c c_hash.c error.c c_math.c

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
_autogen_class_object.h:	$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_object.c
_autogen_class_proc.h:		$(AUTOGEN_METHOD_SRCS)
	$(MAKE_METHOD_TABLE) c_proc.c
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

# HAL
$(BUILD_DIR)/hal.o: $(HAL_DIR)/hal.c $(HAL_DIR)/hal.h
	$(CC) -c $(CFLAGS) -o $(BUILD_DIR)/hal.o $<

# File dependencies.
MRUBYC_H = vm_config.h \
  _autogen_builtin_class.h _autogen_builtin_symbol.h alloc.h \
  c_array.h c_hash.h c_math.h c_numeric.h c_object.h c_proc.h c_range.h \
  c_string.h class.h console.h error.h global.h keyvalue.h load.h symbol.h \
  value.h vm.h

$(BUILD_DIR)/alloc.o: alloc.c vm_config.h alloc.h $(HAL_DIR)/hal.h console.h value.h
$(BUILD_DIR)/c_array.o: c_array.c $(MRUBYC_H) _autogen_class_array.h
$(BUILD_DIR)/c_hash.o: c_hash.c $(MRUBYC_H) _autogen_class_hash.h
$(BUILD_DIR)/c_math.o: c_math.c $(MRUBYC_H)
$(BUILD_DIR)/c_numeric.o: c_numeric.c $(MRUBYC_H) _autogen_class_integer.h _autogen_class_float.h
$(BUILD_DIR)/c_object.o: c_object.c $(MRUBYC_H) _autogen_class_object.h
$(BUILD_DIR)/c_proc.o: c_proc.c $(MRUBYC_H) _autogen_class_proc.h
$(BUILD_DIR)/c_range.o: c_range.c $(MRUBYC_H) _autogen_class_range.h
$(BUILD_DIR)/c_string.o: c_string.c $(MRUBYC_H) _autogen_class_string.h $(AUTOGEN_UNICODE_CASE)
$(BUILD_DIR)/class.o: class.c $(MRUBYC_H)
$(BUILD_DIR)/console.o: console.c $(MRUBYC_H) $(HAL_DIR)/hal.h
$(BUILD_DIR)/error.o: error.c $(MRUBYC_H) _autogen_class_exception.h
$(BUILD_DIR)/global.o: global.c $(MRUBYC_H)
$(BUILD_DIR)/keyvalue.o: keyvalue.c $(MRUBYC_H)
$(BUILD_DIR)/load.o: load.c $(MRUBYC_H)
$(BUILD_DIR)/mrblib.o: mrblib.c
$(BUILD_DIR)/rrt0.o: rrt0.c $(MRUBYC_H) rrt0.h _autogen_class_rrt0.h $(HAL_DIR)/hal.h
$(BUILD_DIR)/symbol.o: symbol.c $(MRUBYC_H) _autogen_class_symbol.h
$(BUILD_DIR)/value.o: value.c $(MRUBYC_H)
$(BUILD_DIR)/vm.o: vm.c $(MRUBYC_H) opcode.h

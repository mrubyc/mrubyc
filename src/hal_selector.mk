#
# mruby/c  src/hal_selector.mk
#
# Copyright (C) 2015-      Kyushu Institute of Technology.
# Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
# Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#

#
# (usage)
#  make MRBC_USE_HAL=../hal/posix
#
ifdef MRBC_USE_HAL_POSIX
  MRBC_USE_HAL = ../hal/posix
endif
ifdef MRBC_USE_HAL_PSOC5LP
  MRBC_USE_HAL = ../hal/psoc5lp
endif
ifdef MRBC_USE_HAL_ESP32
  MRBC_USE_HAL = ../hal/esp32
endif
ifdef MRBC_USE_HAL_PIC24
  MRBC_USE_HAL = ../hal/pic24
endif
ifdef MRBC_USE_HAL_RP2040
  MRBC_USE_HAL = ../hal/rp2040
endif

ifdef MRBC_USE_HAL
  HAL_DIR = $(MRBC_USE_HAL)
  CFLAGS += -I$(MRBC_USE_HAL)
else
  HAL_DIR = ../hal/posix
  CFLAGS += -I$(HAL_DIR)
endif

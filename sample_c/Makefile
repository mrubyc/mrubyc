#
# mruby/c  sample_c/Makefile
#
# Copyright (C) 2015- Kyushu Institute of Technology.
# Copyright (C) 2015- Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

include ../src/hal_selector.mk

TARGETS = sample_scheduler sample_no_scheduler sample_include sample_concurrent sample_myclass
CFLAGS += -I../src -Wall -g
LDFLAGS +=
LIBMRUBYC = ../build/libmrubyc.a

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_CSRCS) -o $@ $^

all: $(TARGETS)

sample_scheduler: sample_scheduler.c $(LIBMRUBYC)
sample_no_scheduler: sample_no_scheduler.c $(LIBMRUBYC)
sample_include: sample_include.c $(LIBMRUBYC)
sample_concurrent: sample_concurrent.c $(LIBMRUBYC)
sample_myclass: sample_myclass.c $(LIBMRUBYC)

clean:
	@rm -rf $(TARGETS) *.o *.dSYM *~

strip:
	strip $(TARGETS)

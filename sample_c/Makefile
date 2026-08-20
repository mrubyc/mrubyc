#
# mruby/c  sample_c/Makefile
#
# Copyright (C) 2015-      Kyushu Institute of Technology.
# Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
# Copyright (C) 2026-      Shimane Institute for Industrial Technology.
#
#  This file is distributed under BSD 3-Clause License.
#

include ../src/hal_selector.mk

TARGETS = sample_scheduler sample_no_scheduler sample_include sample_concurrent sample_myclass sample_task_queue sample_task_queue_timeout
CFLAGS += -I../src -Wall -g
LDFLAGS +=
LIBMRUBYC = ../build/libmrubyc.a
MRBC ?= mrbc

%: %.c
	$(CC) $(CFLAGS) $(EXTRA_CSRCS) -o $@ $< $(LIBMRUBYC) $(LDFLAGS)

all: $(TARGETS)

sample_scheduler: sample_scheduler.c $(LIBMRUBYC)
sample_no_scheduler: sample_no_scheduler.c $(LIBMRUBYC)
sample_include: sample_include.c $(LIBMRUBYC) \
  sample_include_bytecode.c
sample_concurrent: sample_concurrent.c $(LIBMRUBYC)
sample_myclass: sample_myclass.c $(LIBMRUBYC)
sample_task_queue: sample_task_queue.c $(LIBMRUBYC) \
  sample_task_queue_producer.c sample_task_queue_consumer.c
sample_task_queue_timeout: sample_task_queue_timeout.c $(LIBMRUBYC) \
  sample_task_queue_timeout_producer.c sample_task_queue_timeout_consumer.c


sample_include_bytecode.c: sample_include_bytecode.rb
	$(MRBC) --remove-lv -Bmrbbuf -o $@ $<
sample_task_queue_producer.c: sample_task_queue_producer.rb
	$(MRBC) --remove-lv -Bproducer_bytecode -o $@ $<
sample_task_queue_consumer.c: sample_task_queue_consumer.rb
	$(MRBC) --remove-lv -Bconsumer_bytecode -o $@ $<
sample_task_queue_timeout_producer.c: sample_task_queue_timeout_producer.rb
	$(MRBC) --remove-lv -Btimeout_producer_bytecode -o $@ $<
sample_task_queue_timeout_consumer.c: sample_task_queue_timeout_consumer.rb
	$(MRBC) --remove-lv -Btimeout_consumer_bytecode -o $@ $<


clean:
	rm -rf $(TARGETS) *.o *.dSYM *~

strip:
	strip $(TARGETS)

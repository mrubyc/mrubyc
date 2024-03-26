#
# mruby/c  Makefile
#
# Copyright (C) 2015- Kyushu Institute of Technology.
# Copyright (C) 2015- Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

# MRUBY_TAG corresponds to tag or branch of mruby/mruby
MRUBY_TAG = $(shell grep MRUBY_VERSION mrblib/global.rb | sed 's/MRUBY_VERSION *= *"\(.*\)"/\1/')
USER_ID = $(shell id -u)


.PHONY: all mrblib mrubyc_lib mrubyc_bin
all: mrubyc_lib mrubyc_bin

mrblib:
	cd mrblib ; $(MAKE) distclean all

mrubyc_lib:
	cd mrblib ; $(MAKE) all
	cd src ; $(MAKE) all

mrubyc_bin:
	cd sample_c ; $(MAKE) all

.PHONY: clean clean_all
clean:
	cd src ; $(MAKE) clean
	cd sample_c ; $(MAKE) clean

# clean including auto generated files.
clean_all: clean
	cd src ; $(MAKE) clean_all

.PHONY: test setup_test check_tag debug_test
test: test_arm test_host

test_arm:
	make run_test CC=arm-linux-gnueabi-gcc QEMU=qemu-arm-static

test_host:
	make run_test CC=gcc

run_test: check_tag
	docker run --mount type=bind,src=${PWD}/,dst=/work/mrubyc \
	  -e CFLAGS="-DMRBC_USE_HAL_POSIX=1 -DMRBC_USE_MATH=1 -DMRBC_INT64 -DMRBC_SUPPORT_OP_EXT $(CFLAGS)" \
	  -e MRBC="/work/mruby/build/host/bin/mrbc" \
	  mrubyc-dev /bin/sh -c "cd mrblib; make distclean all && cd -; \
	  CC=$(CC) QEMU=$(QEMU) \
	  bundle exec mrubyc-test --every=10 \
	  --mrbc-path=/work/mruby/build/host/bin/mrbc \
	  $(file)"

check_tag:
	$(eval CURRENT_MRUBY_TAG = $(shell docker run mrubyc-dev \
	  /bin/sh -c 'cd /work/mruby && git status | ruby -e"puts STDIN.first.split(\" \")[-1]"'))
	@echo MRUBY_TAG=$(MRUBY_TAG)
	@echo CURRENT_MRUBY_TAG=$(CURRENT_MRUBY_TAG)
	if test "$(CURRENT_MRUBY_TAG)" = "$(MRUBY_TAG)"; \
	then \
	  echo 'Skip setup_test'; \
	else \
	  make setup_test; \
	fi

setup_test:
	docker build -t mrubyc-dev --build-arg MRUBY_TAG=$(MRUBY_TAG) --build-arg USER_ID=$(USER_ID) $(options) .

debug_test:
	gdb $(OPTION) --directory $(shell pwd)/src --args test/tmp/test

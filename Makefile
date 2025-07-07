#
# mruby/c  Makefile
#
# Copyright (C) 2015- Kyushu Institute of Technology.
# Copyright (C) 2015- Shimane IT Open-Innovation Center.
# Copyright (C) 2024- HASUMI Hitoshi.
#
#  This file is distributed under BSD 3-Clause License.
#

USER_ID = $(shell id -u)

.PHONY: all mrblib mrubyc_lib mrubyc_bin
all: mrubyc_lib mrubyc_bin

mrblib:
	cd mrblib   ; $(MAKE) all

mrubyc_lib: mrblib
	cd src      ; $(MAKE) all

mrubyc_bin: mrubyc_lib
	cd sample_c ; $(MAKE) all


.PHONY: clean clean_all
clean:
	cd src      ; $(MAKE) clean
	cd sample_c ; $(MAKE) clean

# clean including auto generated files.
clean_all:
	cd mrblib   ; $(MAKE) clean_all
	cd src      ; $(MAKE) clean_all
	cd sample_c ; $(MAKE) clean


.PHONY: docker_build delete_docker docker_bash

docker_build:
	docker build -t mrubyc-test --no-cache --build-arg USER_ID=$(USER_ID) $(options) .

docker_bash:
	docker run --rm -it -v $(shell pwd):/work/mrubyc mrubyc-test /bin/bash

docker_bash_root:
	docker run -u root --rm -it -v $(shell pwd):/work/mrubyc mrubyc-test /bin/bash

delete_docker:
	docker rmi mrubyc-test

.PHONY: test test_all test_host_gcc test_host_clang test_mips test_arm test_host_gcc_no_libc test_host_clang_no_libc test_mips_no_libc test_arm_no_libc

test: # if platform includes darwin, test_clang, else, test_host_gcc
	@if [ `uname` = "Darwin" ]; then \
		make test_host_clang_no_libc; \
	else \
		make test_host_gcc_no_libc; \
	fi

test_all: test_host_gcc test_host_gcc_no_libc test_host_clang test_host_clang_no_libc test_arm test_arm_no_libc test_mips test_mips_no_libc

test_arm_no_libc:
	docker run \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=arm-linux-gnueabihf \
		-e PICORUBY_NO_LIBC_ALLOC=1 \
		-e RUBY="qemu-arm -L /usr/arm-linux-gnueabihf build/arm-linux-gnueabihf/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_arm:
	docker run \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=arm-linux-gnueabihf \
		-e RUBY="qemu-arm -L /usr/arm-linux-gnueabihf build/arm-linux-gnueabihf/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_gcc_no_libc:
	docker run \
		-e CC=gcc \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e PICORUBY_NO_LIBC_ALLOC=1 \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_gcc:
	docker run \
		-e CC=gcc \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_clang_no_libc:
	docker run \
		-e CC=clang \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e PICORUBY_NO_LIBC_ALLOC=1 \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_clang:
	docker run \
		-e CC=clang \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_mips:
	docker run -e QEMU_LD_PREFIX=/usr/mips-linux-gnu \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=mips-linux-gnu \
		-e RUBY="qemu-mips -L /usr/mips-linux-gnu build/mips-linux-gnu/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_mips_no_libc:
	docker run -e QEMU_LD_PREFIX=/usr/mips-linux-gnu \
		-e PICORUBY_DEBUG=1 \
		-e MRUBY_CONFIG=mips-linux-gnu \
		-e PICORUBY_NO_LIBC_ALLOC=1 \
		-e RUBY="qemu-mips -L /usr/mips-linux-gnu build/mips-linux-gnu/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

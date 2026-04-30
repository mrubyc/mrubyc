#
# mruby/c  Makefile
#
# Copyright (C) 2015-      Kyushu Institute of Technology.
# Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
# Copyright (C) 2026-      Shimane Institute for Industrial Technology.
# Copyright (C) 2024-      HASUMI Hitoshi.
#
#  This file is distributed under BSD 3-Clause License.
#

USER_ID = $(shell id -u)

#
# Targets
#
.PHONY: default all clean clean_all autogen

## Default target
default:
	@cd mrblib;	$(MAKE)
	@cd src;	$(MAKE)
	@cd sample_c;	$(MAKE)

## Rebuild auto-generated files and build target
all:
	@cd mrblib;	$(MAKE) all
	@cd src;	$(MAKE) all
	@cd sample_c;	$(MAKE)

## Clean up object files and editor backups.
clean:
	@cd mrblib;	$(MAKE) clean
	@cd src;	$(MAKE) clean
	@cd sample_c;	$(MAKE) clean

## Remove auto-generated files and intermediate files
clean_all:
	@cd mrblib;	$(MAKE) clean_all
	@cd src;	$(MAKE) clean_all
	@cd sample_c;	$(MAKE) clean

## auto generated files
autogen:
	@cd mrblib;	$(MAKE) all
	@cd src;	$(MAKE) autogen


#
# Tests
#
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

test: autogen # if platform includes darwin, test_clang, else, test_host_gcc
	@if [ `uname` = "Darwin" ]; then \
		make test_host_clang_no_libc; \
	else \
		make test_host_gcc_no_libc; \
	fi

test_all: test_host_gcc test_host_gcc_no_libc test_host_clang test_host_clang_no_libc test_arm test_arm_no_libc test_mips test_mips_no_libc

test_arm_no_libc:
	docker run \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=arm-linux-gnueabihf \
		-e PICORB_NO_LIBC_ALLOC=1 \
		-e RUBY="qemu-arm -L /usr/arm-linux-gnueabihf build/arm-linux-gnueabihf/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_arm:
	docker run \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=arm-linux-gnueabihf \
		-e RUBY="qemu-arm -L /usr/arm-linux-gnueabihf build/arm-linux-gnueabihf/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_gcc_no_libc:
	docker run \
		-e CC=gcc \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e PICORB_NO_LIBC_ALLOC=1 \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_gcc:
	docker run \
		-e CC=gcc \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_clang_no_libc:
	docker run \
		-e CC=clang \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e PICORB_NO_LIBC_ALLOC=1 \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_host_clang:
	docker run \
		-e CC=clang \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=picoruby-test \
		-e RUBY="build/host/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_mips:
	docker run -e QEMU_LD_PREFIX=/usr/mips-linux-gnu \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=mips-linux-gnu \
		-e RUBY="qemu-mips -L /usr/mips-linux-gnu build/mips-linux-gnu/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

test_mips_no_libc:
	docker run -e QEMU_LD_PREFIX=/usr/mips-linux-gnu \
		-e PICORB_DEBUG=1 \
		-e MRUBY_CONFIG=mips-linux-gnu \
		-e PICORB_NO_LIBC_ALLOC=1 \
		-e RUBY="qemu-mips -L /usr/mips-linux-gnu build/mips-linux-gnu/bin/picoruby" \
		--rm -v $(shell pwd):/work/mrubyc mrubyc-test \
		bash -c \
		"rake clean && rake && ruby /work/mrubyc/test/0_runner.rb"

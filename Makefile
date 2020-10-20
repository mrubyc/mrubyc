#
# mruby/c  Makefile
#
# Copyright (C) 2015-2018 Kyushu Institute of Technology.
# Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

# tag or branch name of mruby/mruby
MRUBY_TAG = `grep MRUBY_VERSION mrblib/global.rb | sed 's/MRUBY_VERSION *= *"\(.\+\)"/\1/'`

all: mrubyc_lib mrubyc_bin


mrubyc_lib:
	cd mrblib ; $(MAKE) all
	cd src ; $(MAKE) all

mrubyc_bin:
	cd sample_c ; $(MAKE) all

clean:
	cd mrblib ; $(MAKE) clean
	cd src ; $(MAKE) clean
	cd sample_c ; $(MAKE) clean

package: clean
	@LANG=C ;\
	TARGET="mruby-c_`head -n1 Version`" ;\
	if [ -n "$$MRUBYC_VERSION" ] ;\
		then TARGET="mruby-c_$$MRUBYC_VERSION" ;\
	fi ;\
	echo Making \"$$TARGET.tgz\" ;\
	mkdir -p pkg/$$TARGET ;\
	cp -Rp src doc sample_c sample_ruby auto_test README.md Makefile pkg/$$TARGET ;\
	cd pkg ;\
	tar cfz ../$$TARGET.tgz $$TARGET ;\
	cd .. ;\
	rm -Rf pkg ;\
	echo Done.

.PHONY: test setup_test
test:
	docker run --mount type=bind,src=${PWD}/,dst=/root/mrubyc \
	  -e CFLAGS="-DMRBC_USE_MATH=1 -DMAX_SYMBOLS_COUNT=500 $(CFLAGS)" \
	  mrubyc/mrubyc-test bundle exec mrubyc-test \
	  --every=10 \
	  --mrbc-path=/root/mruby/build/host/bin/mrbc \
	  $(file)

setup_test:
	@echo MRUBY_TAG=$(MRUBY_TAG)
	docker build -t mrubyc/mrubyc-test --build-arg MRUBY_TAG=$(MRUBY_TAG) .

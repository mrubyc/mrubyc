from ruby:3.3.0-slim

RUN apt update && apt -y upgrade
RUN apt install -y \
  bison \
  gcc \
  gcc-arm-linux-gnueabi \
  git \
  make \
  qemu-kvm \
  qemu-user-static

RUN gem update --system

ARG USER_ID
RUN if [ $USER_ID -eq 0 ]; then export USER_ID=1000; fi && useradd -m -u $USER_ID mrubyc
RUN mkdir /work && chown mrubyc /work

USER mrubyc

VOLUME /work/mrubyc
COPY --chown=mrubyc Gemfile /work/mrubyc/
COPY --chown=mrubyc Gemfile.lock /work/mrubyc/

USER root
WORKDIR /work/mrubyc
RUN gem i bundler
RUN bundle install

USER mrubyc
ENV CFLAGS="-DMRBC_USE_MATH=1 -DMAX_SYMBOLS_COUNT=1000"

RUN git clone https://github.com/mruby/mruby /work/mruby
ARG MRUBY_TAG
RUN cd /work/mruby; git fetch --prune; git checkout $MRUBY_TAG; make clean && make

CMD ["bundle", "exec", "mrubyc-test", "-e", "10", "-p", "/work/mruby/build/host/bin/mrbc"]

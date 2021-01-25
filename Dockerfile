from ruby:3.0.0-slim

RUN apt update && apt -y upgrade
RUN apt install -y \
  bison \
  gcc \
  gcc-arm-linux-gnueabi \
  git \
  make \
  qemu \
  qemu-kvm \
  qemu-system-arm

RUN gem update --system

RUN useradd -m -u 1000 mrubyc
RUN mkdir /work && chown mrubyc /work

USER mrubyc

VOLUME /work/mrubyc
COPY --chown=mrubyc Gemfile /work/mrubyc/
COPY --chown=mrubyc Gemfile.lock /work/mrubyc/

USER root
WORKDIR /work/mrubyc
RUN bundle install

USER mrubyc
ENV CFLAGS="-DMRBC_USE_MATH=1 -DMAX_SYMBOLS_COUNT=500"

RUN git clone https://github.com/mruby/mruby /work/mruby
ARG MRUBY_TAG
RUN cd /work/mruby; git fetch --prune; git checkout $MRUBY_TAG; make clean && make

CMD ["bundle", "exec", "mrubyc-test", "-e", "10", "-p", "/work/mruby/build/host/bin/mrbc"]
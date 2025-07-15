FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture armhf

# Update Ubuntu sources.list
RUN echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ jammy main restricted universe multiverse" > /etc/apt/sources.list && \
    echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ jammy-updates main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ jammy-security main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports jammy main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports jammy-updates main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports jammy-security main restricted universe multiverse" >> /etc/apt/sources.list

RUN apt-get update && apt-get install -y --no-install-recommends \
    ruby \
    qemu-user \
    qemu-user-static \
    qemu-system-arm \
    binfmt-support \
    build-essential \
    gcc \
    clang \
    libc++-dev \
    libc++abi-dev \
    g++-arm-linux-gnueabihf \
    libc6-dev-armhf-cross \
    libc6:armhf \
    g++-mips-linux-gnu \
    libc6-dev-mips-cross \
    gcc-mips-linux-gnu \
    binutils-mips-linux-gnu \
    git \
    cmake \
    make \
    libssl-dev \
    libssl-dev:armhf \
    ca-certificates \
    tzdata \
    pkg-config \
    file \
    vim \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN gem install rake bundler

ARG USER_ID=1000
RUN useradd -m -u "$USER_ID" mrubyc

RUN mkdir /work && chown mrubyc /work

USER mrubyc
RUN git clone --depth 1 --recursive -b master \
    https://github.com/picoruby/picoruby /work/picoruby

WORKDIR /work/picoruby
ENV MRUBY_CONFIG=picoruby-test
RUN rake --trace
ENV MRUBY_CONFIG=arm-linux-gnueabihf
RUN rake --trace
ENV MRUBY_CONFIG=mips-linux-gnu
RUN rake --trace
ENV MRUBY_CONFIG=picoruby-test

VOLUME /work/mrubyc

RUN cd /work/picoruby/mrbgems/picoruby-mrubyc/lib; \
    rm -rf mrubyc; \
    ln -s /work/mrubyc mrubyc

WORKDIR /work/picoruby

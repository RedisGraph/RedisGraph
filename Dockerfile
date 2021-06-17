# BUILD redisfab/redisgraph:${VERSION}-${ARCH}-${OSNICK}

ARG REDIS_VER=6.2.4

# OSNICK=bionic|stretch|buster
ARG OSNICK=buster

# OS=debian:buster-slim|debian:stretch-slim|ubuntu:bionic
ARG OS=debian:buster-slim

# ARCH=x64|arm64v8|arm32v7
ARG ARCH=x64

ARG PACK=0
ARG TEST=0

#----------------------------------------------------------------------------------------------
FROM redisfab/redis:${REDIS_VER}-${ARCH}-${OSNICK} AS redis
# Build based on ${OS} (i.e., 'builder'), redis files are copies from 'redis'
FROM ${OS} AS builder

# Re-introducude arguments to this image
ARG OSNICK
ARG OS
ARG ARCH
ARG REDIS_VER

RUN echo "Building for ${OSNICK} (${OS}) for ${ARCH} [with Redis ${REDIS_VER}]"

WORKDIR /build

COPY --from=redis /usr/local/ /usr/local/

ADD ./ /build

# Set up a build environment
RUN ./deps/readies/bin/getpy3
RUN ./sbin/system-setup.py
RUN set -ex ;\
    if [ -e /usr/bin/apt-get ]; then \
        apt-get update -qq; \
        apt-get upgrade -yqq; \
        rm -rf /var/cache/apt; \
    fi
RUN if [ -e /usr/bin/yum ]; then \
        yum update -y; \
        rm -rf /var/cache/yum; \
    fi

RUN if [ ! -z $(command -v apt-get) ]; then \
        locale-gen --purge en_US.UTF-8 ;\
        dpkg-reconfigure -f noninteractive locales ;\
    fi

ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

RUN bash -l -c make -j`nproc`

ARG PACK
ARG TEST

RUN set -ex ;\
    if [ "$TEST" = "1" ]; then bash -l -c "TEST= make test"; fi
RUN set -ex ;\
    mkdir -p bin/artifacts ;\
    if [ "$PACK" = "1" ]; then bash -l -c "make package"; fi

#----------------------------------------------------------------------------------------------
FROM redisfab/redis:${REDIS_VER}-${ARCH}-${OSNICK}

ARG OSNICK
ARG OS
ARG ARCH
ARG REDIS_VER
ARG PACK

ENV LIBDIR /usr/lib/redis/modules

WORKDIR /data

RUN mkdir -p $LIBDIR

COPY --from=builder /build/bin/artifacts/ /var/opt/redislabs/artifacts
COPY --from=builder /build/src/redisgraph.so $LIBDIR

RUN if [ ! -z $(command -v apt-get) ]; then apt-get -qq update; apt-get -q install -y libgomp1; fi
RUN if [ ! -z $(command -v yum) ]; then yum install -y libgomp; fi

EXPOSE 6379
CMD ["redis-server", "--loadmodule", "/usr/lib/redis/modules/redisgraph.so"]

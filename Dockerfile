# BUILD redisfab/redisgraph:${VERSION}-${ARCH}-${OSNICK}

ARG REDIS_VER=6.0.5

# OSNICK=bionic|stretch|buster
ARG OSNICK=buster

# OS=debian:buster-slim|debian:stretch-slim|ubuntu:bionic
ARG OS=debian:buster-slim

# ARCH=x64|arm64v8|arm32v7
ARG ARCH=x64

#----------------------------------------------------------------------------------------------
FROM redisfab/redis:${REDIS_VER}-${ARCH}-${OSNICK} AS redis
# Build based on ${OS} (i.e., 'builder'), redis files are copies from 'redis'
FROM ${OS} AS builder

# Re-introducude arguments to this image
ARG OSNICK
ARG OS
ARG ARCH
ARG REDIS_VER

RUN echo "Building for ${OSNICK} (${OS}) for ${ARCH}" 

WORKDIR /build

COPY --from=redis /usr/local/ /usr/local/

ADD ./ /build

# Set up a build environment
RUN set -ex ;\
	apt-get -qq update ;\
	apt-get install -y --no-install-recommends ca-certificates wget git;\
	apt-get install -y --no-install-recommends python python-pip python-psutil ;\
	apt-get install -y --no-install-recommends build-essential cmake m4 automake autoconf libtool peg libgomp1 ;\
	python -m pip install wheel ;\
	python -m pip install setuptools --upgrade ;\
	python -m pip install git+https://github.com/Grokzen/redis-py-cluster.git@master ;\
	python -m pip install git+https://github.com/RedisLabsModules/RLTest.git@master ;\
	python -m pip install git+https://github.com/RedisLabs/RAMP@master ;\
	python -m pip install -r tests/requirements.txt

RUN make

ARG TEST=0
ARG PACK=0

RUN set -ex ;\
	if [ "$TEST" = "1" ]; then TEST= make test; fi
RUN set -ex ;\
	if [ "$PACK" = "1" ]; then \
		python -m RAMP.ramp pack -m ramp.yml -o "build/redisgraph.{os}-{architecture}.{semantic_version}.zip" src/redisgraph.so ;\
	fi

#---------------------------------------------------------------------------------------------- 
FROM redisfab/redis:${REDIS_VER}-${ARCH}-${OSNICK}

ENV LIBDIR /usr/lib/redis/modules

WORKDIR /data

RUN set -ex ;\
    mkdir -p "$LIBDIR" ;\
    apt-get -qq update ;\
    apt-get install -y --no-install-recommends libgomp1

COPY --from=builder /build/src/redisgraph.so "$LIBDIR"

EXPOSE 6379
CMD ["redis-server", "--loadmodule", "/usr/lib/redis/modules/redisgraph.so"]

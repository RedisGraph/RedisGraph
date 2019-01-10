FROM redis:latest as builder

ENV DEPS "automake peg libtool autoconf python python-setuptools python-pip wget build-essential cmake m4"

# Set up a build environment
RUN set -ex;\
    deps="$DEPS";\
    apt-get update;\
    apt-get install -y --no-install-recommends $deps;\
    pip install rmtest;\
    pip install redisgraph;

# Build the source
ADD ./ /redisgraph

# Build RedisGraph
WORKDIR /redisgraph
RUN set -ex;\
    make clean; \
    make;

# Package the runner
FROM redis:latest
ENV LIBDIR /usr/lib/redis/modules
WORKDIR /data
RUN set -ex;\
    mkdir -p "$LIBDIR";

COPY --from=builder /redisgraph/src/redisgraph.so "$LIBDIR"

EXPOSE 6379
CMD ["redis-server", "--loadmodule", "/usr/lib/redis/modules/redisgraph.so"]

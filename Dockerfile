FROM redis:latest as builder

ENV DEPS "python python-setuptools python-pip wget build-essential cmake"

# Set up a build environment
RUN set -ex;\
    deps="$DEPS";\
    apt-get update;\
	apt-get install -y --no-install-recommends $deps;\
    pip install rmtest;\
    pip install redisgraph;

# Build the source
ADD ./ /redisgraph

# Temporaraly individualy build GraphBLAS
WORKDIR /redisgraph/deps/GraphBLAS
RUN set -ex;\
    cmake; \
    make install;

# Build RedisGraph
WORKDIR /redisgraph
RUN set -ex;\
    make clean; \
    make all -j 4;

# Package the runner
FROM redis:latest
ENV LIBDIR /var/lib/redis/modules
WORKDIR /data
RUN set -ex;\
    mkdir -p "$LIBDIR";\
    apt-get update;\
    apt-get install libgomp1;

COPY --from=builder /usr/local/lib/libgraphblas* /usr/lib/
COPY --from=builder /usr/local/include/GraphBLAS.h /usr/local/include/GraphBLAS.h
COPY --from=builder /redisgraph/src/redisgraph.so "$LIBDIR"

CMD ["redis-server", "--loadmodule", "/var/lib/redis/modules/redisgraph.so"]

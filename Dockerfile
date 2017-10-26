FROM redis:latest as builder

ENV DEPS "python python-setuptools python-pip wget build-essential"

# Set up a build environment
RUN set -ex;\
    deps="$DEPS";\
    apt-get update;\
	apt-get install -y --no-install-recommends $deps;\
    pip install rmtest;\
    pip install redisgraph;

# Build the source
ADD ./ /redisgraph
WORKDIR /redisgraph
RUN set -ex;\
    make clean; \
    make all -j 4; \
    make test;

# Package the runner
FROM redis:latest
ENV LIBDIR /var/lib/redis/modules
WORKDIR /data
RUN set -ex;\
    mkdir -p "$LIBDIR";
COPY --from=builder /redisgraph/src/redisgraph.so "$LIBDIR"

CMD ["redis-server", "--loadmodule", "/var/lib/redis/modules/redisgraph.so"]
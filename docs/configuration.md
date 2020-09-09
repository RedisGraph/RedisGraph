# Run-time Configuration

RedisGraph supports a few run-time configuration options that can be defined when loading the module. In the future more options will be added.

## Passing configuration options on module load

Passing configuration options is done by appending arguments after the `--loadmodule` argument when starting a server from the command line or after the `loadmodule` directive in a Redis config file. For example:

In redis.conf:

```
loadmodule ./redisgraph.so OPT1 OPT2
```

From the command line:

```
$ redis-server --loadmodule ./redisgraph.so OPT1 OPT2
```

# RedisGraph configuration options

## THREAD_COUNT

The number of threads in RedisGraph's thread pool. This is equivalent to the maximum number of queries that can be processed concurrently.

### Default

`THREAD_COUNT` defaults to the system's processor count.

### Example

```
$ redis-server --loadmodule ./redisgraph.so THREAD_COUNT 4
```

---

## CACHE_SIZE

The max number of queries for RedisGraph to cache. When a new query is encountered and the cache is full, meaning the cache has reached the size of `CACHE_SIZE`, it will evict the least recently used (LRU) entry.

### Default

`CACHE_SIZE` default value is 25.

### Example

```
$ redis-server --loadmodule ./redisgraph.so CACHE_SIZE 10
```

---

## OMP_THREAD_COUNT

The maximum number of threads that OpenMP may use for computation. These threads are used for parallelizing GraphBLAS computations, so may be considered to control concurrency within the execution of individual queries.

### Default

`OMP_THREAD_COUNT` is defined by GraphBLAS by default.

### Example

```
$ redis-server --loadmodule ./redisgraph.so OMP_THREAD_COUNT 1
```

---

## MAINTAIN_TRANSPOSED_MATRICES

If enabled, RedisGraph will maintain transposed copies of relationship matrices. This improves the performance of traversing edges from destination to source, but has a higher memory overhead and requires more write operations when updating edges.

### Default

`MAINTAIN_TRANSPOSED_MATRICES` is on by default (config value of `yes`).

### Example

```
$ redis-server --loadmodule ./redisgraph.so MAINTAIN_TRANSPOSED_MATRICES no
```

# Query Configurations

Some configurations may be set per query in the form of additional arguments after the query string. All per-query configurations are off by default unless using a language-specific client, which may establish its own defaults.

## Query Timeout

The query flag `timeout` allows the user to specify the maximum runtime allowed for a query in milliseconds. This configuration can only be set for read queries to avoid leaving the graph in an inconsistent state.

`timeout` may still return partial results followed by an error message indicating the timeout.

### Example

Retrieve all paths in a graph with a timeout of 1000 milliseconds.

```
GRAPH.QUERY wikipedia "MATCH p=()-[*]->() RETURN p" timeout 1000
```

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

---

## MAX_QUEUED_QUERIES

Setting the maximum number of queued queries allows the server to reject incoming queries with the error message `Max pending queries exceeded`. This reduces the memory overhead of pending queries on an overloaded server and avoids congestion when the server processes its backlog of queries.

This configuration can be set when the module loads or at runtime.

### Default

`MAX_QUEUED_QUERIES` is effectively unlimited by default (config value of `UINT64_MAX`).

### Example

```
$ redis-server --loadmodule ./redisgraph.so MAX_QUEUED_QUERIES 500

$ redis-cli GRAPH.CONFIG SET MAX_QUEUED_QUERIES 500
```

---

## TIMEOUT

Timeout is a flag that specifies the maximum runtime for read queries in milliseconds. This configuration will not be respected by write queries, to avoid leaving the graph in an inconsistent state.

### Default

`TIMEOUT` is off by default (config value of `0`).

### Example

```
$ redis-server --loadmodule ./redisgraph.so TIMEOUT 1000
```

---

## QUERY_MEM_CAPACITY

Setting the memory capacity of a query allows the server to kill queries that are consuming too much memory and return with the error message `Query's mem consumption exceeded capacity`. This helps to avoid scenarios when the server becomes unresponsive due to an unbounded query exhausting system resources.

The configuration argument is the maximum number of bytes that can be allocated by any single query.

This configuration can be set when the module loads or at runtime.

### Default

`QUERY_MEM_CAPACITY` is unlimited by default; this default can be restored by setting `QUERY_MEM_CAPACITY` to zero or a negative value.

### Example

```
$ redis-server --loadmodule ./redisgraph.so QUERY_MEM_CAPACITY 1048576 // 1 megabyte limit

$ redis-cli GRAPH.CONFIG SET QUERY_MEM_CAPACITY 1048576
```

---

## NODE_CREATION_BUFFER

The node creation buffer is the number of new nodes that can be created without resizing matrices. When set to 0, the matrices will have no extra space and every node creation will increase the graph dimensions by 1.

Reducing this value will reduce memory consumption, but cause performance degradation due to the increased frequency of matrix resizes.

Conversely, increasing it might improve performance for write-heavy workloads but will increase memory consumption.

If the passed argument was not a power of 2, it will be rounded to the next-greatest power of 2 to improve memory alignment.

This configuration can only be set when the module loads.

### Default

`NODE_CREATION_BUFFER` is 16,384 by default.

### Minimum

The minimum value for `NODE_CREATION_BUFFER` is 128. Values lower than this will be accepted as arguments, but will internally be converted to 128.

### Example

```
$ redis-server --loadmodule ./redisgraph.so NODE_CREATION_BUFFER 200
```

# Query Configurations

Some configurations may be set per query in the form of additional arguments after the query string. All per-query configurations are off by default unless using a language-specific client, which may establish its own defaults.

## Query Timeout

The query flag `timeout` allows the user to specify a timeout as described in [TIMEOUT](#TIMEOUT) for a single query.

### Example

Retrieve all paths in a graph with a timeout of 1000 milliseconds.

```
GRAPH.QUERY wikipedia "MATCH p=()-[*]->() RETURN p" timeout 1000
```

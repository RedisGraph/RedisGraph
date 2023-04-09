---
title: "Configuration Parameters"
linkTitle: "Configuration"
weight: 3
description: >
    RedisGraph supports multiple module configuration parameters. Some of these parameters can only be set at load-time, while other parameters can be set either on load-time or on run-time.
---

## Setting configuration parameters on module load

Setting configuration parameters at load-time is done by appending arguments after the `--loadmodule` argument when starting a server from the command line or after the `loadmodule` directive in a Redis config file. For example:

In [redis.conf](/docs/manual/config/):

```sh
loadmodule ./redisgraph.so [OPT VAL]...
```

From the [Redis CLI](/docs/manual/cli/), using the [MODULE LOAD](/commands/module-load/) command:

```
127.0.0.6379> MODULE LOAD redisgraph.so [OPT VAL]...
```

From the command line:

```sh
$ redis-server --loadmodule ./redisgraph.so [OPT VAL]...
```

## Setting configuration parameters at run-time (for supported parameters)

RedisGraph exposes the `GRAPH.CONFIG` endpoint to allowing for the setting and retrieval of configuration parameters at run-time.

To set the value of a configuration parameter at run-time (for supported parameters), simply run:

```sh
GRAPH.CONFIG SET OPT1 VAL1
```

Similarly, current configuration parameter values can be retrieved using:

```sh
GRAPH.CONFIG GET OPT1
GRAPH.CONFIG GET *
```

Values set using `GRAPH.CONFIG SET` are not persisted after server restart.

## RedisGraph configuration parameters

The following table summarizes which configuration parameters can be set at module load-time and which can be set on run-time:

| Configuration Parameter                                      | Load-time          | Run-time             |
| :-------                                                     | :-----             | :-----------         |
| [THREAD_COUNT](#thread_count)                                | :white_check_mark: | :white_large_square: |
| [CACHE_SIZE](#cache_size)                                    | :white_check_mark: | :white_large_square: |
| [OMP_THREAD_COUNT](#omp_thread_count)                        | :white_check_mark: | :white_large_square: |
| [NODE_CREATION_BUFFER](#node_creation_buffer)                | :white_check_mark: | :white_large_square: |
| [MAX_QUEUED_QUERIES](#max_queued_queries)                    | :white_check_mark: | :white_check_mark:   |
| [TIMEOUT](#timeout) (deprecated in RedisGraph v2.10)         | :white_check_mark: | :white_check_mark:   |
| [TIMEOUT_MAX](#timeout_max) (since RedisGraph v2.10)         | :white_check_mark: | :white_check_mark:   |
| [TIMEOUT_DEFAULT](#timeout_default) (since RedisGraph v2.10) | :white_check_mark: | :white_check_mark:   |
| [RESULTSET_SIZE](#resultset_size)                            | :white_check_mark: | :white_check_mark:   |
| [QUERY_MEM_CAPACITY](#query_mem_capacity)                    | :white_check_mark: | :white_check_mark:   |
| [VKEY_MAX_ENTITY_COUNT](#vkey_max_entity_count)              | :white_check_mark: | :white_check_mark:   |
| [EFFECTS_THRESHOLD](#effects_threshold)                      | :white_check_mark: | :white_check_mark:   |

---

### THREAD_COUNT

The number of threads in RedisGraph's thread pool. This is equivalent to the maximum number of queries that can be processed concurrently.

#### Default

`THREAD_COUNT` defaults to the system's hardware threads (logical cores).

#### Example

```
$ redis-server --loadmodule ./redisgraph.so THREAD_COUNT 4
```

---

### CACHE_SIZE

The max number of queries for RedisGraph to cache. When a new query is encountered and the cache is full, meaning the cache has reached the size of `CACHE_SIZE`, it will evict the least recently used (LRU) entry.

#### Default

`CACHE_SIZE` default value is 25.

#### Example

```
$ redis-server --loadmodule ./redisgraph.so CACHE_SIZE 10
```

---

### OMP_THREAD_COUNT

The maximum number of threads that OpenMP may use for computation per query. These threads are used for parallelizing GraphBLAS computations, so may be considered to control concurrency within the execution of individual queries.

#### Default

`OMP_THREAD_COUNT` is defined by GraphBLAS.

#### Example

```
$ redis-server --loadmodule ./redisgraph.so OMP_THREAD_COUNT 1
```

---

### NODE_CREATION_BUFFER

The node creation buffer is the number of new nodes that can be created without resizing matrices. For example, when set to 16,384, the matrices will have extra space for 16,384 nodes upon creation. Whenever the extra space is depleted, the matrices' size will increase by 16,384.

Reducing this value will reduce memory consumption, but cause performance degradation due to the increased frequency of matrix resizes.

Conversely, increasing it might improve performance for write-heavy workloads but will increase memory consumption.

If the passed argument was not a power of 2, it will be rounded to the next-greatest power of 2 to improve memory alignment.

#### Default

`NODE_CREATION_BUFFER` is 16,384.

#### Minimum

The minimum value for `NODE_CREATION_BUFFER` is 128. Values lower than this will be accepted as arguments, but will internally be converted to 128.

#### Example

```
$ redis-server --loadmodule ./redisgraph.so NODE_CREATION_BUFFER 200
```

---

### MAX_QUEUED_QUERIES

Setting the maximum number of queued queries allows the server to reject incoming queries with the error message `Max pending queries exceeded`. This reduces the memory overhead of pending queries on an overloaded server and avoids congestion when the server processes its backlog of queries.

#### Default

`MAX_QUEUED_QUERIES` is effectively unlimited (config value of `UINT64_MAX`).

#### Example

```
$ redis-server --loadmodule ./redisgraph.so MAX_QUEUED_QUERIES 500

$ redis-cli GRAPH.CONFIG SET MAX_QUEUED_QUERIES 500
```

---

### TIMEOUT

(Deprecated in RedisGraph v2.10 It is recommended to use `TIMEOUT_MAX` and `TIMEOUT_DEFAULT` instead)

The `TIMEOUT` configuration parameter specifies the default maximal execution time for read queries, in milliseconds. Write queries do not timeout.

When a read query execution time exceeds the maximal execution time, the query is aborted and the query reply is `(error) Query timed out`.

The `TIMEOUT` query parameter of the `GRAPH.QUERY`, `GRAPH.RO_QUERY`, and `GRAPH.PROFILE` commands can be used to override this value.

#### Default

- Before RedisGraph v2.10: `TIMEOUT` is off (set to `0`).
- Since RedisGraph v2.10: `TIMEOUT` is not specified; `TIMEOUT_MAX` and `TIMEOUT_DEFAULT` are specified instead.

#### Example

```
$ redis-server --loadmodule ./redisgraph.so TIMEOUT 1000
```

---

### TIMEOUT_MAX

(Since RedisGraph v2.10)

The `TIMEOUT_MAX` configuration parameter specifies the maximum execution time for both read and write queries, in milliseconds.

The `TIMEOUT` query parameter value of the `GRAPH.QUERY`, `GRAPH.RO_QUERY`, and `GRAPH.PROFILE` commands cannot exceed the `TIMEOUT_MAX` value (the command would abort with a `(error) The query TIMEOUT parameter value cannot exceed the TIMEOUT_MAX configuration parameter value` reply). Similarly, the `TIMEOUT_DEFAULT` configuration parameter cannot exceed the `TIMEOUT_MAX` value.

When a query execution time exceeds the maximal execution time, the query is aborted and the query reply is `(error) Query timed out`. For a write query - any change to the graph is undone (which may take additional time).

#### Default

- Before RedisGraph v2.10: unspecified and unsupported.
- Since RedisGraph v2.10: `TIMEOUT_MAX` is off (set to `0`).

#### Example

```
$ redis-server --loadmodule ./redisgraph.so TIMEOUT_MAX 1000
```

---

### TIMEOUT_DEFAULT

(Since RedisGraph v2.10)

The `TIMEOUT_DEFAULT` configuration parameter specifies the default maximal execution time for both read and write queries, in milliseconds.

For a given query, this default maximal execution time can be overridden by the `TIMEOUT` query parameter of the `GRAPH.QUERY`, `GRAPH.RO_QUERY`, and `GRAPH.PROFILE` commands. However, a query execution time cannot exceed `TIMEOUT_MAX`.

#### Default

- Before RedisGraph v2.10: unspecified and unsupported.
- Since RedisGraph v2.10: `TIMEOUT_DEFAULT` is equal to `TIMEOUT_MAX` (set to `0`).

#### Example

```
$ redis-server --loadmodule ./redisgraph.so TIMEOUT_MAX 2000 TIMEOUT_DEFAULT 1000
```

---


### RESULTSET_SIZE

Result set size is a limit on the number of records that should be returned by any query. This can be a valuable safeguard against incurring a heavy IO load while running queries with unknown results.

#### Default

`RESULTSET_SIZE` is unlimited (negative config value).

#### Example

```
127.0.0.1:6379> GRAPH.CONFIG SET RESULTSET_SIZE 3
OK
127.0.0.1:6379> GRAPH.QUERY G "UNWIND range(1, 5) AS x RETURN x"
1) 1) "x"
2) 1) 1) (integer) 1
   2) 1) (integer) 2
   3) 1) (integer) 3
3) 1) "Cached execution: 0"
   2) "Query internal execution time: 0.445790 milliseconds"
```

---

### QUERY_MEM_CAPACITY

Setting the memory capacity of a query allows the server to kill queries that are consuming too much memory and return with the error message `Query's mem consumption exceeded capacity`. This helps to avoid scenarios when the server becomes unresponsive due to an unbounded query exhausting system resources.

The configuration argument is the maximum number of bytes that can be allocated by any single query.

#### Default

`QUERY_MEM_CAPACITY` is unlimited; this default can be restored by setting `QUERY_MEM_CAPACITY` to zero or a negative value.

#### Example

```
$ redis-server --loadmodule ./redisgraph.so QUERY_MEM_CAPACITY 1048576 // 1 megabyte limit

$ redis-cli GRAPH.CONFIG SET QUERY_MEM_CAPACITY 1048576
```

---

### VKEY_MAX_ENTITY_COUNT

To lower the time Redis is blocked when replicating large graphs,
RedisGraph serializes the graph in a number of virtual keys.

One virtual key is created for every N graph entities,
where N is the value defined by this configuration.

This configuration can be set when the module loads or at runtime.

#### Default

`VKEY_MAX_ENTITY_COUNT` is 100,000.

---

## Query Configurations

### Query Timeout

- Before RedisGraph v2.10, or if `TIMEOUT_DEFAULT` and `TIMEOUT_MAX` are not specified:

  `TIMEOUT` allows overriding the `TIMEOUT` configuration parameter for a single read query. Write queries do not timeout.

- Since RedisGraph v2.10, if either `TIMEOUT_DEFAULT` or `TIMEOUT_MAX` are specified:

  `TIMEOUT` allows overriding the `TIMEOUT_DEFAULT` configuration parameter value for a single `GRAPH.QUERY`, `GRAPH.RO_QUERY`, or `GRAPH.PROFILE` command. The `TIMEOUT` value cannot exceed the `TIMEOUT_MAX` value (the command would abort with a `(error) The query TIMEOUT parameter value cannot exceed the TIMEOUT_MAX configuration parameter value` reply).

#### Example

Retrieve all paths in a graph with a timeout of 500 milliseconds.

```
GRAPH.QUERY wikipedia "MATCH p=()-[*]->() RETURN p" TIMEOUT 500
```

---

### EFFECTS_THRESHOLD

Replicate modification via effect when average modification time > `EFFECTS_THRESHOLD`

#### Default

`EFFECTS_THRESHOLD` is 300 μs.

#### Example

Assume `MATCH (n) WHERE n.id < 100 SET n.v = n.v + 1` updated 5 nodes
and the query total execution time is 5ms, the average modification time is:
total execution time / number of changes:  5ms / 5 = 1ms.
if the average modification time is greater then `EFFECTS_THRESHOLD` the query
will be replicated to both replicas and AOF as a graph effect otherwise the original
query will be replicated.

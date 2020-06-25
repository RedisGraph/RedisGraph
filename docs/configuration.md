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

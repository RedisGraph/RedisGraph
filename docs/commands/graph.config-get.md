Retrieves the current value of a RedisGraph configuration parameter.

RedisGraph configuration parameters are detailed [here](/docs/stack/graph/configuration).

`*` can be used to retrieve the value of all RedisGraph configuration parameters.

```
127.0.0.1:6379> graph.config get *
 1) 1) "TIMEOUT"
    2) (integer) 0
 2) 1) "CACHE_SIZE"
    2) (integer) 25
 3) 1) "ASYNC_DELETE"
    2) (integer) 1
 4) 1) "OMP_THREAD_COUNT"
    2) (integer) 8
 5) 1) "THREAD_COUNT"
    2) (integer) 8
 6) 1) "RESULTSET_SIZE"
    2) (integer) -1
 7) 1) "VKEY_MAX_ENTITY_COUNT"
    2) (integer) 100000
 8) 1) "MAX_QUEUED_QUERIES"
    2) (integer) 4294967295
 9) 1) "QUERY_MEM_CAPACITY"
    2) (integer) 0
10) 1) "DELTA_MAX_PENDING_CHANGES"
    2) (integer) 10000
11) 1) "NODE_CREATION_BUFFER"
    2) (integer) 16384
```

```
127.0.0.1:6379> graph.config get TIMEOUT
1) "TIMEOUT"
2) (integer) 0
```

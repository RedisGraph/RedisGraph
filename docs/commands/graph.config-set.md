Set the value of a RedisGraph configuration parameter.

Values set using `GRAPH.CONFIG SET` are not persisted after server restart.

RedisGraph configuration parameters are detailed [here](/docs/stack/graph/configuration).

Note: As detailed in the link above, not all RedisGraph configuration parameters can be set at run-time.

```
127.0.0.1:6379> graph.config get TIMEOUT
1) "TIMEOUT"
2) (integer) 0
127.0.0.1:6379> graph.config set TIMEOUT 10000
OK
127.0.0.1:6379> graph.config get TIMEOUT
1) "TIMEOUT"
2) (integer) 10000
```

```
127.0.0.1:6379> graph.config set THREAD_COUNT 10
(error) This configuration parameter cannot be set at run-time
```

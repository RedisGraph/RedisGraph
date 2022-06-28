Retrieves or updates a RedisGraph configuration.
Arguments: `GET/SET, <config name> [value]`
`value` should only be specified in `SET` contexts, while `*` may be substituted for an explicit `config name` if all configurations should be returned.
Only run-time configurations may be `SET`, though all configurations may be retrieved.
```sh
127.0.0.1:6379> GRAPH.CONFIG SET RESULTSET_SIZE 1000
OK
127.0.0.1:6379> GRAPH.CONFIG GET RESULTSET_SIZE
1) "RESULTSET_SIZE"
2) (integer) 1000
```

# RedisGraph Administration Guide

RedisGraph doesn't require any configuration to work, but there are a few things worth noting when running RedisGraph on top of Redis.

## Persistence

RedisGraph supports both RDB and AOF based persistence. For a pure RDB set-up, nothing special is needed beyond the standard Redis RDB configuration.

### AOF Persistence

While RedisGraph supports working with AOF based persistence, it **does not support** "classic AOF" mode, which uses AOF rewriting. Instead, it only supports AOF with RDB preamble mode. In this mode, rewriting the AOF log just creates an RDB file, which is appended to. 

To enable AOF persistence with RedisGraph, add the two following lines to your redis.conf:

```
appendonly yes
aof-use-rdb-preamble yes
``` 

## Replication

RedisGraph supports replication inherently, and using a master-replica set-up, you can use replicas for high availability. On top of that, replicas can be used for querying, to load-balance read traffic. 

## Cluster Support

When creating multiple graphs in RedisGraph running on a Redis cluster, the graphs will be distributed across the cluster. RedisGraph does not support sharded graph.

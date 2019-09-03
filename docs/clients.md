# Client Libraries

The full functionality of RedisGraph is available through `redis-cli` and the Redis API, but a number of client libraries have been written to improve abstractions and allow for a more natural experience in a project's native language. Additionally, these clients take advantage of some RedisGraph features that may reduce network throughput in some circumstances.

## Currently-available Libraries

| Project | Language | License | Author | URL |
| ------- | -------- | ------- | ------ | --- |
| redisgraph-py | Python | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph-py) |
| JRedisGraph | Java | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/JRedisGraph) |
| redisgraph-rb | Ruby | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph-rb) |
| redisgraph-go | Go | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph-go) |
| redisgraph.js | JavaScript | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph.js) |
| ioredisgraph | JavaScript | ISC | [Jonah](https://github.com/Jonahss) | [GitHub](https://github.com/Jonahss/ioredisgraph) |
| php-redis-graph | PHP | MIT | [KJDev](https://github.com/kjdev) | [GitHub](https://github.com/kjdev/php-redis-graph) |

## Implementing a client

Information on some of the tasks involved in writing a RedisGraph client can be found in the [Client Specification](client_spec.md).
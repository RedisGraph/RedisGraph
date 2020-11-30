# Client Libraries

The full functionality of RedisGraph is available through `redis-cli` and the Redis API, but a number of client libraries have been written to improve abstractions and allow for a more natural experience in a project's native language. Additionally, these clients take advantage of some RedisGraph features that may reduce network throughput in some circumstances.

## Currently-available Libraries

| Project                 | Language   | License | Author                                     | URL                                   |
| ----------------------- | ---------- | ------- | ------------------------------------------ | ------------------------------------- |
| redisgraph-py           | Python     | BSD     | [Redis Labs][redisgraph-py-author]         | [GitHub][redisgraph-py-url]           |
| JRedisGraph             | Java       | BSD     | [Redis Labs][JRedisGraph-author]           | [GitHub][JRedisGraph-url]             |
| redisgraph-rb           | Ruby       | BSD     | [Redis Labs][redisgraph-rb-author]         | [GitHub][redisgraph-rb-url]           |
| redisgraph-go           | Go         | BSD     | [Redis Labs][redisgraph-go-author]         | [GitHub][redisgraph-go-url]           |
| redisgraph.js           | JavaScript | BSD     | [Redis Labs][redisgraph.js-author]         | [GitHub][redisgraph.js-url]           |
| ioredisgraph            | JavaScript | ISC     | [Jonah][ioredisgraph-author]               | [GitHub][ioredisgraph-url]            |
| @hydre/rgraph           | JavaScript | MIT     | [Sceat][rgraph-author]                     | [Github][rgraph-url]                  |
| php-redis-graph         | PHP        | MIT     | [KJDev][php-redis-graph-author]            | [GitHub][php-redis-graph-url]         |
| redisgraph_php          | PHP        | MIT     | [jpbourbon][redisgraph_php-author]         | [GitHub][redisgraph_php-url]          |
| redislabs-redisgraph-php| PHP        | MIT     | [mkorkmaz][redislabs-redisgraph-php-author]| [GitHub][redislabs-redisgraph-php-url]|
| redisgraph-ex           | Elixir     | MIT     | [crflynn][redisgraph-ex-author]            | [GitHub][redisgraph-ex-url]           |
| redisgraph-rs           | RUST       | MIT     | [malte-v][redisgraph-rs-author]            | [GitHub][redisgraph-rs-url]           |

[redisgraph-py-author]: https://redislabs.com
[redisgraph-py-url]: https://github.com/RedisGraph/redisgraph-py

[JRedisGraph-author]: https://redislabs.com
[JRedisGraph-url]: https://github.com/RedisGraph/JRedisGraph

[redisgraph-rb-author]: https://redislabs.com
[redisgraph-rb-url]: https://github.com/RedisGraph/redisgraph-rb

[redisgraph-go-author]: https://redislabs.com
[redisgraph-go-url]: https://github.com/RedisGraph/redisgraph-go

[redisgraph.js-author]: https://redislabs.com
[redisgraph.js-url]: https://github.com/RedisGraph/redisgraph.js

[rgraph-author]: https://github.com/Sceat
[rgraph-url]: https://github.com/HydreIO/rgraph

[ioredisgraph-author]: https://github.com/Jonahss
[ioredisgraph-url]: https://github.com/Jonahss/ioredisgraph

[php-redis-graph-author]: https://github.com/kjdev
[php-redis-graph-url]: https://github.com/kjdev/php-redis-graph

[redisgraph_php-author]: https://github.com/jpbourbon
[redisgraph_php-url]: https://github.com/jpbourbon/redisgraph_php

[redislabs-redisgraph-php-author]: https://github.com/mkorkmaz
[redislabs-redisgraph-php-url]: https://github.com/mkorkmaz/redislabs-redisgraph-php

[redisgraph-ex-author]: https://github.com/crflynn
[redisgraph-ex-url]: https://github.com/crflynn/redisgraph-ex

[redisgraph-rs-author]: https://github.com/malte-v
[redisgraph-rs-url]: https://github.com/malte-v/redisgraph-rs

## Implementing a client

Information on some of the tasks involved in writing a RedisGraph client can be found in the [Client Specification](client_spec.md).

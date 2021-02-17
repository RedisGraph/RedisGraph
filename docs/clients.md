# RedisGraph Client Libraries

The full functionality of RedisGraph is available through `redis-cli` and the Redis API.  [RedisInsight](https://redislabs.com/redis-enterprise/redis-insight/) is a visual tool that provides capabilities to design, develop and optimize into a single easy-to-use environment, and has built-in support for RedisGraph.  
In addition there are severeal client libraries to improve abstractions and allow for a more natural experience in a project's native language. Additionally, these clients take advantage of some RedisGraph features that may reduce network throughput in some circumstances.

## Currently available Libraries

| Project                                                   | Language   | License | Author                                      | Stars                                                          | 
| --------------------------------------------------------- | ---------- | ------- | ------------------------------------------- | -------------------------------------------------------------- |
| [redisgraph-py][redisgraph-py-url]                        | Python     | BSD     | [Redis Labs][redisgraph-py-author]          | [![redisgraph-py-stars]][redisgraph-py-url]                    |  
| [JRedisGraph][JRedisGraph-url]                            | Java       | BSD     | [Redis Labs][JRedisGraph-author]            | [![JRedisGraph-stars]][JRedisGraph-url]                        |
| [redisgraph-rb][redisgraph-rb-url]                        | Ruby       | BSD     | [Redis Labs][redisgraph-rb-author]          | [![redisgraph-rb-stars]][redisgraph-rb-url]                    |
| [redisgraph-go][redisgraph-go-url]                        | Go         | BSD     | [Redis Labs][redisgraph-go-author]          | [![redisgraph-go-stars]][redisgraph-go-url]                    |
| [redisgraph.js][redisgraph.js-url]                        | JavaScript | BSD     | [Redis Labs][redisgraph.js-author]          | [![redisgraph.js-stars]][redisgraph.js-url]                    |
| [ioredisgraph][ioredisgraph-url]                          | JavaScript | ISC     | [Jonah][ioredisgraph-author]                | [![ioredisgraph-stars]][ioredisgraph-url]                      |
| [@hydre/rgraph][rgraph-url]                               | JavaScript | MIT     | [Sceat][rgraph-author]                      | [![rgraph-stars]][rgraph-url]                                  |
| [redis-modules-sdk][redis-modules-sdk-url] | TypeScript | BSD-3-Clause | [Dani Tseitlin][redis-modules-sdk-author] | [![redis-modules-sdk-stars]][redis-modules-sdk-url]                                |
| [php-redis-graph][php-redis-graph-url]                    | PHP        | MIT     | [KJDev][php-redis-graph-author]             | [![php-redis-graph-stars]][php-redis-graph-url]                |
| [redislabs-redisgraph-php][redislabs-redisgraph-php-url]  | PHP        | MIT     | [mkorkmaz][redislabs-redisgraph-php-author] | [![redislabs-redisgraph-php-stars]][redislabs-redisgraph-php-url] |
| [redisgraph_php][redisgraph_php-url]                      | PHP        | MIT     | [jpbourbon][redisgraph_php-author]          | [![redisgraph_php-stars]][redisgraph_php-url]                  |
| [redisgraph-ex][redisgraph-ex-url]                        | Elixir     | MIT     | [crflynn][redisgraph-ex-author]             | [![redisgraph-ex-stars]][redisgraph-ex-url]                    |
| [redisgraph-rs][redisgraph-rs-url]                        | RUST       | MIT     | [malte-v][redisgraph-rs-author]             | [![redisgraph-rs-stars]][redisgraph-rs-url]                    |
| [redis_graph][redis_graph-url]                            | RUST       | BSD     | [tompro][redis_graph-author]                | [![redis_graph-stars]][redis_graph-url]                        |
| [NRedisGraph][NRedisGraph-url]                            | C#         | BSD     | [tombatron][NRedisGraph-author]             | [![NRedisGraph-stars]][redis_graph-url]                        |
| [RedisGraphDotNet.Client][RedisGraphDotNet.Client-url]    | C#         | BSD     | [Sgawrys][RedisGraphDotNet.Client-author]   | [![RedisGraphDotNet.Client-stars]][RedisGraphDotNet.Client-url] |


[redisgraph-py-author]: https://redislabs.com
[redisgraph-py-url]: https://github.com/RedisGraph/redisgraph-py
[redisgraph-py-stars]: https://img.shields.io/github/stars/RedisGraph/redisgraph-py.svg?style=social&amp;label=Star&amp;maxAge=2592000

[JRedisGraph-author]: https://redislabs.com
[JRedisGraph-url]: https://github.com/RedisGraph/JRedisGraph
[JRedisGraph-stars]: https://img.shields.io/github/stars/RedisGraph/JRedisGraph.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redisgraph-rb-author]: https://redislabs.com
[redisgraph-rb-url]: https://github.com/RedisGraph/redisgraph-rb
[redisgraph-rb-stars]: https://img.shields.io/github/stars/RedisGraph/redisgraph-rb.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redisgraph-go-author]: https://redislabs.com
[redisgraph-go-url]: https://github.com/RedisGraph/redisgraph-go
[redisgraph-go-stars]: https://img.shields.io/github/stars/RedisGraph/redisgraph-go.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redisgraph.js-author]: https://redislabs.com
[redisgraph.js-url]: https://github.com/RedisGraph/redisgraph.js
[redisgraph.js-stars]: https://img.shields.io/github/stars/RedisGraph/redisgraph.js.svg?style=social&amp;label=Star&amp;maxAge=2592000

[rgraph-author]: https://github.com/Sceat
[rgraph-url]: https://github.com/HydreIO/rgraph
[rgraph-stars]: https://img.shields.io/github/stars/HydreIO/rgraph.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redis-modules-sdk-author]: https://github.com/danitseitlin
[redis-modules-sdk-url]: https://github.com/danitseitlin/redis-modules-sdk
[redis-modules-sdk-stars]: https://img.shields.io/github/stars/danitseitlin/redis-modules-sdk.svg?style=social&amp;label=Star&amp;maxAge=2592000

[ioredisgraph-author]: https://github.com/Jonahss
[ioredisgraph-url]: https://github.com/Jonahss/ioredisgraph
[ioredisgraph-stars]: https://img.shields.io/github/stars/Jonahss/ioredisgraph.svg?style=social&amp;label=Star&amp;maxAge=2592000

[php-redis-graph-author]: https://github.com/kjdev
[php-redis-graph-url]: https://github.com/kjdev/php-redis-graph
[php-redis-graph-stars]: https://img.shields.io/github/stars/kjdev/php-redis-graph.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redisgraph_php-author]: https://github.com/jpbourbon
[redisgraph_php-url]: https://github.com/jpbourbon/redisgraph_php
[redisgraph_php-stars]: https://img.shields.io/github/stars/jpbourbon/redisgraph_php.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redislabs-redisgraph-php-author]: https://github.com/mkorkmaz
[redislabs-redisgraph-php-url]: https://github.com/mkorkmaz/redislabs-redisgraph-php
[redislabs-redisgraph-php-stars]: https://img.shields.io/github/stars/mkorkmaz/redislabs-redisgraph-php.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redisgraph-ex-author]: https://github.com/crflynn
[redisgraph-ex-url]: https://github.com/crflynn/redisgraph-ex
[redisgraph-ex-stars]: https://img.shields.io/github/stars/crflynn/redisgraph-ex.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redisgraph-rs-author]: https://github.com/malte-v
[redisgraph-rs-url]: https://github.com/malte-v/redisgraph-rs
[redisgraph-rs-stars]: https://img.shields.io/github/stars/malte-v/redisgraph-rs.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redis_graph-author]: https://github.com/tompro
[redis_graph-url]: https://github.com/tompro/redis_graph
[redis_graph-stars]: https://img.shields.io/github/stars/tompro/redis_graph.svg?style=social&amp;label=Star&amp;maxAge=2592000

[NRedisGraph-author]: https://github.com/tombatron
[NRedisGraph-url]: https://github.com/tombatron/NRedisGraph
[NRedisGraph-stars]: https://img.shields.io/github/stars/tombatron/NRedisGraph.svg?style=social&amp;label=Star&amp;maxAge=2592000

[RedisGraphDotNet.Client-author]: https://github.com/Sgawrys
[RedisGraphDotNet.Client-url]: https://github.com/Sgawrys/RedisGraphDotNet.Client
[RedisGraphDotNet.Client-stars]: https://img.shields.io/github/stars/Sgawrys/RedisGraphDotNet.Client.svg?style=social&amp;label=Star&amp;maxAge=2592000


## Implementing a client

Information on some of the tasks involved in writing a RedisGraph client can be found in the [Client Specification](client_spec.md).

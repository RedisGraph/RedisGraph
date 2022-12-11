[![Release](https://img.shields.io/github/release/RedisGraph/RedisGraph.svg?sort=semver)](https://github.com/RedisGraph/RedisGraph/releases/latest)
[![CircleCI](https://circleci.com/gh/RedisGraph/RedisGraph/tree/master.svg?style=svg)](https://circleci.com/gh/RedisGraph/RedisGraph/tree/master)
[![Dockerhub](https://img.shields.io/badge/dockerhub-redislabs%2Fredisgraph-blue)](https://hub.docker.com/r/redislabs/redisgraph/tags/)
[![Codecov](https://codecov.io/gh/RedisGraph/RedisGraph/branch/master/graph/badge.svg)](https://codecov.io/gh/RedisGraph/RedisGraph)
[![Bounty](https://img.shields.io/bountysource/team/redislabsmodules/activity)](https://app.bountysource.com/teams/redislabsmodules/issues)

# RedisGraph
## A graph database module for Redis
[![Forum](https://img.shields.io/badge/Forum-RedisGraph-blue)](https://forum.redislabs.com/c/modules/redisgraph)
[![Discord](https://img.shields.io/discord/697882427875393627?style=flat-square)](https://discord.gg/gWBRT6P)

RedisGraph is the first queryable [Property Graph](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc) database to use [sparse matrices](https://en.wikipedia.org/wiki/Sparse_matrix) to represent the [adjacency matrix](https://en.wikipedia.org/wiki/Adjacency_matrix) in graphs and [linear algebra](http://faculty.cse.tamu.edu/davis/GraphBLAS.html) to query the graph.

Primary features:
* Adopting the [Property Graph Model](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc)
  * Nodes (vertices) and Relationships (edges) that may have attributes
  * Nodes can have multiple labels
  * Relationships have a relationship type
* Graphs represented as sparse adjacency matrices
* [OpenCypher](http://www.opencypher.org/) with proprietary extensions as a query language
  * Queries are translated into linear algebra expressions

To see RedisGraph in action, visit [Demos](https://github.com/RedisGraph/RedisGraph/tree/master/demo).
To read the docs, visit [redis.io](https://redis.io/docs/stack/graph/).

## Quickstart

1. [Trying RedisGraph](#trying-redisgraph)
2. [Docker](#docker)
3. [Build](#building)
4. [Start](#loading-redisgraph-into-redis)
5. [Use from any client](#using-redisgraph)

## Trying RedisGraph

To try RedisGraph, either use the RedisGraph Docker image, or [create a free Redis Cloud Essentials account](https://redislabs.com/try-free/) to get a RedisGraph instance in the cloud.

## Docker

To quickly tryout RedisGraph, launch an instance using docker:

```sh
docker run -p 6379:6379 -it --rm redis/redis-stack-server:latest
```

### Give it a try

Once loaded you can interact with RedisGraph using redis-cli.

Here we'll quickly create a small graph representing a subset of motorcycle riders and teams taking part in the MotoGP league,
once created we'll start querying our data.

### With `redis-cli`

The format of results through `redis-cli` is described in [the RedisGraph documentation](https://redis.io/docs/stack/graph/design/result_structure/).

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY MotoGP "CREATE (:Rider {name:'Valentino Rossi'})-[:rides]->(:Team {name:'Yamaha'}), (:Rider {name:'Dani Pedrosa'})-[:rides]->(:Team {name:'Honda'}), (:Rider {name:'Andrea Dovizioso'})-[:rides]->(:Team {name:'Ducati'})"
1) 1) Labels added: 2
   2) Nodes created: 6
   3) Properties set: 6
   4) Relationships created: 3
   5) "Query internal execution time: 0.399000 milliseconds"
```

Now that our MotoGP graph is created, we can start asking questions, for example:
Who's riding for team Yamaha?

```sh
127.0.0.1:6379> GRAPH.QUERY MotoGP "MATCH (r:Rider)-[:rides]->(t:Team) WHERE t.name = 'Yamaha' RETURN r.name, t.name"
1) 1) "r.name"
   2) "t.name"
2) 1) 1) "Valentino Rossi"
      2) "Yamaha"
3) 1) "Query internal execution time: 0.625399 milliseconds"
```

How many riders represent team Ducati?

```sh
127.0.0.1:6379> GRAPH.QUERY MotoGP "MATCH (r:Rider)-[:rides]->(t:Team {name:'Ducati'}) RETURN count(r)"
1) 1) "count(r)"
2) 1) 1) (integer) 1
3) 1) "Query internal execution time: 0.624435 milliseconds"
```

## Building

### Compiling

Requirements:

* The RedisGraph repository: `git clone --recurse-submodules -j8 https://github.com/RedisGraph/RedisGraph.git`

* On Ubuntu Linux, run: `apt-get install build-essential cmake m4 automake peg libtool autoconf python3`

* On OS X, verify that `homebrew` is installed and run: `brew install cmake m4 automake peg libtool autoconf`.
    * The version of Clang that ships with the OS X toolchain does not support OpenMP, which is a requirement for RedisGraph. One way to resolve this is to run `brew install gcc g++` and follow the on-screen instructions to update the symbolic links. Note that this is a system-wide change - setting the environment variables for `CC` and `CXX` will work if that is not an option.

To build, run `make` in the project's directory.

Congratulations! You can find the compiled binary at `src/redisgraph.so`.

### Running tests

First, install required Python packages by running ```pip install -r requirements.txt``` from the ```tests``` directory.

If you've got ```redis-server``` in PATH, just invoke ```make test```.

Otherwise, invoke ```REDIS_SERVER=<redis-server-location> make test```.

For more verbose output, run ```make test V=1```.

### Building in a docker

The RedisGraph build system runs within docker. For detailed instructions on building, please [see here](docs/docker-examples/README.md).

## Loading RedisGraph into Redis

RedisGraph is hosted by [Redis](https://redis.io), so you'll first have to load it as a Module to a Redis server: running [Redis v5.0.7 or above](https://redis.io/download).

We recommend having Redis load RedisGraph during startup by adding the following to your redis.conf file:

```
loadmodule /path/to/module/src/redisgraph.so
```

In the line above, replace `/path/to/module/src/redisgraph.so` with the actual path to RedisGraph's library.
If Redis is running as a service, you must ensure that the `redis` user (default) has the necessary file/folder permissions
to access `redisgraph.so`.

Alternatively, you can have Redis load RedisGraph using the following command line argument syntax:

```sh
~/$ redis-server --loadmodule /path/to/module/src/redisgraph.so
```

Lastly, you can also use the [`MODULE LOAD`](http://redis.io/commands/module-load) command. Note, however, that `MODULE LOAD` is a dangerous command and may be blocked/deprecated in the future due to security considerations.

Once you've successfully loaded RedisGraph your Redis log should have lines similar to:

```
...
30707:M 20 Jun 02:08:12.314 * Module 'graph' loaded from <redacted>/src/redisgraph.so
...
```

If the server fails to launch with output similar to:

```
# Module /usr/lib/redis/modules/redisgraph.so failed to load: libgomp.so.1: cannot open shared object file: No such file or directory
# Can't load module from /usr/lib/redis/modules/redisgraph.so: server aborting
```

The system is missing the run-time dependency OpenMP. This can be installed on Ubuntu with `apt-get install libgomp1`, on RHEL/CentOS with `yum install libgomp`, and on OSX with `brew install libomp`.

## Using RedisGraph

You can call RedisGraph's commands from any Redis client.

### With `redis-cli`

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY social "CREATE (:person {name: 'roi', age: 33, gender: 'male', status: 'married'})"
```

### With any other client

You can interact with RedisGraph using your client's ability to send raw Redis commands.

Depending on your client of choice, the exact method for doing that may vary.

#### Python example

This code snippet shows how to use RedisGraph with raw Redis commands from Python via
[redis-py](https://github.com/andymccurdy/redis-py):

```python
import redis

r = redis.StrictRedis()
reply = r.execute_command('GRAPH.QUERY', 'social', "CREATE (:person {name:'roi', age:33, gender:'male', status:'married'})")
```

### Client libraries

Some languages have client libraries that provide support for RedisGraph's commands:

| Project                                                   | Language   | License | Author                                      | Stars                                                             | Comment    |
| --------------------------------------------------------- | ---------- | ------- | ------------------------------------------- | ----------------------------------------------------------------- | ---------- |
| [Jedis][Jedis-url]                                        | Java       | MIT     | [Redis][Jedis-author]                       | [![Jedis-stars]][Jedis-url]                                       |
| [redis-modules-java][redis-modules-java-url]              | Java       | Apache 2.0 | [dengliming][redis-modules-java-author]               | [![redis-modules-java-stars]][redis-modules-java-url]                       |
| [redisgraph-py][redisgraph-py-url]                        | Python     | BSD     | [Redis Labs][redisgraph-py-author]          | [![redisgraph-py-stars]][redisgraph-py-url]                       |
| [JRedisGraph][JRedisGraph-url]                            | Java       | BSD     | [Redis Labs][JRedisGraph-author]            | [![JRedisGraph-stars]][JRedisGraph-url]                           | Deprecated |
| [redisgraph-rb][redisgraph-rb-url]                        | Ruby       | BSD     | [Redis Labs][redisgraph-rb-author]          | [![redisgraph-rb-stars]][redisgraph-rb-url]                       |
| [redisgraph-go][redisgraph-go-url]                        | Go         | BSD     | [Redis Labs][redisgraph-go-author]          | [![redisgraph-go-stars]][redisgraph-go-url]                       |
| [rueidis][rueidis-url]                                    | Go         | Apache 2.0 | [Rueian][rueidis-author]          | [![rueidis-stars]][rueidis-url]                       |
| [redisgraph.js][redisgraph.js-url]                        | JavaScript | BSD     | [Redis Labs][redisgraph.js-author]          | [![redisgraph.js-stars]][redisgraph.js-url]                       |
| [ioredisgraph][ioredisgraph-url]                          | JavaScript | ISC     | [Jonah][ioredisgraph-author]                | [![ioredisgraph-stars]][ioredisgraph-url]                         |
| [@hydre/rgraph][rgraph-url]                               | JavaScript | MIT     | [Sceat][rgraph-author]                      | [![rgraph-stars]][rgraph-url]                                     |
| [redis-modules-sdk][redis-modules-sdk-url] | TypeScript | BSD-3-Clause | [Dani Tseitlin][redis-modules-sdk-author] | [![redis-modules-sdk-stars]][redis-modules-sdk-url]                           |
| [php-redis-graph][php-redis-graph-url]                    | PHP        | MIT     | [KJDev][php-redis-graph-author]             | [![php-redis-graph-stars]][php-redis-graph-url]                   |
| [redislabs-redisgraph-php][redislabs-redisgraph-php-url]  | PHP        | MIT     | [mkorkmaz][redislabs-redisgraph-php-author] | [![redislabs-redisgraph-php-stars]][redislabs-redisgraph-php-url] |
| [redisgraph_php][redisgraph_php-url]                      | PHP        | MIT     | [jpbourbon][redisgraph_php-author]          | [![redisgraph_php-stars]][redisgraph_php-url]                     |
| [redisgraph-ex][redisgraph-ex-url]                        | Elixir     | MIT     | [crflynn][redisgraph-ex-author]             | [![redisgraph-ex-stars]][redisgraph-ex-url]                       |
| [redisgraph-rs][redisgraph-rs-url]                        | RUST       | MIT     | [malte-v][redisgraph-rs-author]             | [![redisgraph-rs-stars]][redisgraph-rs-url]                       |
| [redis_graph][redis_graph-url]                            | RUST       | BSD     | [tompro][redis_graph-author]                | [![redis_graph-stars]][redis_graph-url]                           |
| [NRedisGraph][NRedisGraph-url]                            | C#         | BSD     | [tombatron][NRedisGraph-author]             | [![NRedisGraph-stars]][redis_graph-url]                           |
| [RedisGraphDotNet.Client][RedisGraphDotNet.Client-url]    | C#         | BSD     | [Sgawrys][RedisGraphDotNet.Client-author]   | [![RedisGraphDotNet.Client-stars]][RedisGraphDotNet.Client-url]   |
| [RedisGraph.jl][RedisGraph.jl-url]                        | Julia      | MIT     | [xyxel][RedisGraph.jl-author]               | [![RedisGraph.jl-stars]][RedisGraph.jl-url]                       |


[Jedis-author]: https://redis.com
[Jedis-url]: https://github.com/redis/jedis
[Jedis-stars]: https://img.shields.io/github/stars/redis/jedis.svg?style=social&amp;label=Star&amp;maxAge=2592000

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

[rueidis-url]: https://github.com/rueian/rueidis
[rueidis-author]: https://github.com/rueian
[rueidis-stars]: https://img.shields.io/github/stars/rueian/rueidis.svg?style=social&amp;label=Star&amp;maxAge=2592000

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

[RedisGraph.jl-author]: https://github.com/xyxel
[RedisGraph.jl-url]: https://github.com/xyxel/RedisGraph.jl
[RedisGraph.jl-stars]: https://img.shields.io/github/stars/xyxel/RedisGraph.jl.svg?style=social&amp;label=Star&amp;maxAge=2592000

[redis-modules-java-author]: https://github.com/dengliming
[redis-modules-java-url]: https://github.com/dengliming/redis-modules-java
[redis-modules-java-stars]: https://img.shields.io/github/stars/dengliming/redis-modules-java.svg?style=social&amp;label=Star&amp;maxAge=2592000

## Documentation

Read the docs at [redisgraph.io](http://redisgraph.io).

## Mailing List / Forum

Got questions? Feel free to ask at the [RedisGraph forum](https://forum.redislabs.com/c/modules/redisgraph).

## License

Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or the Server Side Public License v1 (SSPLv1). See [LICENSE](LICENSE.txt).

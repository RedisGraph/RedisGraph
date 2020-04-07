[![Release](https://img.shields.io/github/release/RedisGraph/RedisGraph.svg?sort=semver)](https://github.com/RedisGraph/RedisGraph/releases/latest)
[![CircleCI](https://circleci.com/gh/RedisGraph/RedisGraph/tree/master.svg?style=svg)](https://circleci.com/gh/RedisGraph/RedisGraph/tree/master)
[![Docker Cloud Build Status](https://img.shields.io/docker/cloud/build/redislabs/redisgraph.svg)](https://hub.docker.com/r/redislabs/redisgraph/builds/)
[![Mailing List](https://img.shields.io/badge/Mailing%20List-RedisGraph-blue)](https://groups.google.com/forum/#!forum/redisgraph)
[![Gitter](https://badges.gitter.im/RedisLabs/RedisGraph.svg)](https://gitter.im/RedisLabs/RedisGraph?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

# RedisGraph - A graph database module for Redis

RedisGraph is the first queryable [Property Graph](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc) database to use [sparse matrices](https://en.wikipedia.org/wiki/Sparse_matrix) to represent the [adjacency matrix](https://en.wikipedia.org/wiki/Adjacency_matrix) in graphs and [linear algebra](http://faculty.cse.tamu.edu/davis/GraphBLAS.html) to query the graph.

Primary features:
* Adopting the [Property Graph Model](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc)
  * Nodes (vertices) and Relationships (edges) that may have attributes
  * Nodes that can be labeled
  * Relationships have a relationship type
* Graphs represented as sparse adjacency matrices
* [Cypher](http://www.opencypher.org/) as query language
  * Cypher queries translated into linear algebra expressions

To see RedisGraph in action, visit [Demos](https://github.com/RedisGraph/RedisGraph/tree/master/demo).

## Quickstart

1. [Docker](#docker)
2. [Build](#building)
3. [Start](#loading-redisgraph-into-redis)
4. [Use from any client](#using-redisgraph)

## Docker

To quickly tryout RedisGraph, launch an instance using docker:

```sh
docker run -p 6379:6379 -it --rm redislabs/redisgraph
```

### Give it a try

Once loaded you can interact with RedisGraph using redis-cli.

Here we'll quickly create a small graph representing a subset of motorcycle riders and teams taking part in the MotoGP league,
once created we'll start querying our data.

### With `redis-cli`

The format of results through `redis-cli` is described in [the RedisGraph documentation](https://oss.redislabs.com/redisgraph/result_structure).

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

* On Ubuntu Linux, run: `apt-get install build-essential cmake m4 automake peg libtool autoconf`

* On OS X, verify that `homebrew` is installed and run: `brew install cmake m4 automake peg libtool autoconf`.
    * The version of Clang that ships with the OS X toolchain does not support OpenMP, which is a requirement for RedisGraph. One way to resolve this is to run `brew install gcc g++` and follow the on-screen instructions to update the symbolic links. Note that this is a system-wide change - setting the environment variables for `CC` and `CPP` will work if that is not an option.

To build, run `make` in the project's directory.

Congratulations! You can find the compiled binary at `src/redisgraph.so`.

### Running tests

First, install required Python packages by running ```pip install -r requirements.txt``` from the ```tests``` directory.

If you've got ```redis-server``` in PATH, just invoke ```make test```.

Otherwise, invoke ```REDIS_SERVER=<redis-server-location> make test```.

For more verbose output, run ```make test V=1```.

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
reply = r.execute_command('GRAPH.QUERY', 'social', "CREATE (:person {name:'roi', age:33, gender:'male', status:'married')")
```

### Client libraries

Some languages have client libraries that provide support for RedisGraph's commands:

| Project         | Language   | License | Author                             | URL                           |
| --------------- | ---------- | ------- | ---------------------------------- | ----------------------------- |
| redisgraph-py   | Python     | BSD     | [Redis Labs][redisgraph-py-author] | [GitHub][redisgraph-py-url]   |
| JRedisGraph     | Java       | BSD     | [Redis Labs][JRedisGraph-author]   | [GitHub][JRedisGraph-url]     |
| redisgraph-rb   | Ruby       | BSD     | [Redis Labs][redisgraph-rb-author] | [GitHub][redisgraph-rb-url]   |
| redisgraph-go   | Go         | BSD     | [Redis Labs][redisgraph-go-author] | [GitHub][redisgraph-go-url]   |
| redisgraph.js   | JavaScript | BSD     | [Redis Labs][redisgraph.js-author] | [GitHub][redisgraph.js-url]   |
| ioredisgraph    | JavaScript | ISC     | [Jonah][ioredisgraph-author]       | [GitHub][ioredisgraph-url]    |
| @hydre/rgraph   | JavaScript | MIT     | [Sceat][rgraph-author]             | [Github][rgraph-url]          |
| php-redis-graph | PHP        | MIT     | [KJDev][php-redis-graph-author]    | [GitHub][php-redis-graph-url] |
| redisgraph_php  | PHP        | MIT     | [jpbourbon][redisgraph_php-author] | [GitHub][redisgraph_php-url]  |
| redisgraph-ex   | Elixir     | MIT     | [crflynn][redisgraph-ex-author]    | [GitHub][redisgraph-ex-url]   |
| redisgraph-rs   | RUST       | MIT     | [malte-v][redisgraph-rs-author]    | [GitHub][redisgraph-rs-url]   |

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

[redisgraph-ex-author]: https://github.com/crflynn
[redisgraph-ex-url]: https://github.com/crflynn/redisgraph-ex

[redisgraph-rs-author]: https://github.com/malte-v
[redisgraph-rs-url]: https://github.com/malte-v/redisgraph-rs


## Documentation

Read the docs at [redisgraph.io](http://redisgraph.io).

## Mailing List / Forum

Got questions? Feel free to ask at the [RediGraph mailing list](https://groups.google.com/forum/#!forum/redisgraph).

## License

Redis Source Available License Agreement - see [LICENSE](LICENSE)

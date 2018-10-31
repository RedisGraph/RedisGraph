[![CircleCI](https://circleci.com/gh/RedisLabsModules/redis-graph/tree/master.svg?style=svg)](https://circleci.com/gh/RedisLabsModules/redis-graph/tree/master)

# RedisGraph - A graph database module for Redis

RedisGraph is the first queryable [Property Graph](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc) database to use [sparse matrices](https://en.wikipedia.org/wiki/Sparse_matrix) to represent adjacency in graphs and [linear algebra](http://faculty.cse.tamu.edu/davis/GraphBLAS.html) to query the graph.

Primary features:
* Adopting the [Property Graph Model](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc)
  * Nodes (vertices) and Relationships (edges) that may have attributes
  * Nodes that can be labeled
  * Relationships have a relationship type
* Graphs represented as sparse adjacency matrices
* [Cypher](http://www.opencypher.org/) as query language
  * Cypher queries translated into linear algebra expressions

To see RedisGraph in action, visit [Demos](https://github.com/RedisLabsModules/redis-module-graph/tree/master/demo).

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

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY MotoGP "CREATE (:Rider {name:'Valentino Rossi'})-[:rides]->(:Team {name:'Yamaha'}), (:Rider {name:'Dani Pedrosa'})-[:rides]->(:Team {name:'Honda'}), (:Rider {name:'Andrea Dovizioso'})-[:rides]->(:Team {name:'Ducati'})"
1) (empty list or set)
2) 1) Labels added: 2
   2) Nodes created: 6
   3) Properties set: 6
   4) Relationships created: 3
   5) "Query internal execution time: 0.399000 milliseconds"
```

Now that our MotoGP graph is created, we can start asking questions, for example:
Who's riding for team Yamaha?

```sh
127.0.0.1:6379> GRAPH.QUERY MotoGP "MATCH (r:Rider)-[:rides]->(t:Team) WHERE t.name = 'Yamaha' RETURN r,t"
1) 1) 1) "r.name"
      2) "t.name"
   2) 1) "Valentino Rossi"
      2) "Yamaha"
2) 1) "Query internal execution time: 0.122000 milliseconds"
```

How many riders represent team Ducati?

```sh
127.0.0.1:6379> GRAPH.QUERY MotoGP "MATCH (r:Rider)-[:rides]->(t:Team {name:'Ducati'}) RETURN count(r)"
1) 1) 1) "count(r)"
   2) 1) "1.000000"
2) 1) "Query internal execution time: 0.129000 milliseconds"
```

## Building

### Linux Ubuntu 16.04

Requirements:

* The RedisGraph repository: `git clone https://github.com/RedisLabsModules/redis-graph.git`
* The build-essential and cmake packages: `apt-get install build-essential cmake`

To build, run `make` in the project's directory

Congratulations! You can find the compiled binary at `src/redisgraph.so`.

## Loading RedisGraph into Redis

RedisGraph is hosted by [Redis](https://redis.io), so you'll first have to load it as a Module to a Redis server: running [Redis v4.0 or above](https://redis.io/download).

We recommend having Redis load RedisGraph during startup by adding the following to your redis.conf file:

```
loadmodule /path/to/module/src/redisgraph.so
```

In the line above, replace `/path/to/module/src/redisgraph.so` with the actual path to RedisGraph's library.

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

| Project | Language | License | Author | URL |
| ------- | -------- | ------- | ------ | --- |
| redisgraph-py | Python | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph-py) |
| JRedisGraph | Java | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/JRedisGraph) |
| redisgraph-rb | Ruby | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph-rb) |
| redisgraph-go | Go | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph-go) |
| redisgraph.js | JavaScript | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/RedisLabs/redisgraph.js) |
| php-redis-graph | PHP | MIT | [KJDev](https://github.com/kjdev) | [GitHub](https://github.com/kjdev/php-redis-graph) |

## Documentation

Read the docs at [redisgraph.io](http://redisgraph.io).

## License

Apache 2.0 with Commons Clause - see [LICENSE](LICENSE)

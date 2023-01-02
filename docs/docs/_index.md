---
title: RedisGraph
linkTitle: Graph
description: A Graph database built on Redis
type: docs
---

[![Discord](https://img.shields.io/discord/697882427875393627?style=flat-square)](https://discord.gg/gWBRT6P)
[![Github](https://img.shields.io/static/v1?label=&message=repository&color=5961FF&logo=github)](https://github.com/RedisGraph/RedisGraph/)

RedisGraph is a graph database built on Redis. This graph database uses [GraphBlas](http://faculty.cse.tamu.edu/davis/GraphBLAS.html) under the hood for its [sparse adjacency matrix](https://en.wikipedia.org/wiki/Adjacency_matrix) graph representation.

## Primary features

* Adopting the [Property Graph Model](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc)
  * Nodes (vertices) and Relationships (edges) that may have attributes
  * Nodes can have multiple labels
  * Relationships have a relationship type
* Graphs represented as sparse adjacency matrices
* [OpenCypher](http://www.opencypher.org/) with proprietary extensions as a query language
  * Queries are translated into linear algebra expressions

To see RedisGraph in action, visit [Demos](https://github.com/RedisGraph/RedisGraph/tree/master/demo).
To read the docs, visit [redis.io](https://redis.io/docs/stack/graph/).

## Docker

To quickly try out RedisGraph, launch an instance using docker:

```
docker run -p 6379:6379 -it --rm redis/redis-stack-server
```

### Give it a try

After you load RedisGraph, you can interact with it using `redis-cli`.

Here we'll quickly create a small graph representing a subset of motorcycle riders and teams taking part in the MotoGP championship. Once created, we'll start querying our data.

### With `redis-cli`

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY MotoGP "CREATE (:Rider {name:'Valentino Rossi'})-[:rides]->(:Team {name:'Yamaha'}), (:Rider {name:'Dani Pedrosa'})-[:rides]->(:Team {name:'Honda'}), (:Rider {name:'Andrea Dovizioso'})-[:rides]->(:Team {name:'Ducati'})"
1) 1) "Labels added: 2"
   2) "Nodes created: 6"
   3) "Properties set: 6"
   4) "Relationships created: 3"
   5) "Cached execution: 0"
   6) "Query internal execution time: 0.399000 milliseconds"
```

Now that our MotoGP graph is created, we can start asking questions. For example:
Who's riding for team Yamaha?

```sh
127.0.0.1:6379> GRAPH.QUERY MotoGP "MATCH (r:Rider)-[:rides]->(t:Team) WHERE t.name = 'Yamaha' RETURN r.name, t.name"
1) 1) "r.name"
   2) "t.name"
2) 1) 1) "Valentino Rossi"
      2) "Yamaha"
3) 1) "Cached execution: 0"
   2) "Query internal execution time: 0.625399 milliseconds"
```

How many riders represent team Ducati?

```sh
127.0.0.1:6379> GRAPH.QUERY MotoGP "MATCH (r:Rider)-[:rides]->(t:Team {name:'Ducati'}) RETURN count(r)"
1) 1) "count(r)"
2) 1) 1) (integer) 1
3) 1) "Cached execution: 0"
   2) "Query internal execution time: 0.624435 milliseconds"
```

## Building

Requirements:

* The RedisGraph repository: `git clone --recurse-submodules -j8 https://github.com/RedisGraph/RedisGraph.git`

* On Ubuntu Linux, run: `apt-get install build-essential cmake m4 automake peg libtool autoconf python3`

* On OS X, verify that `homebrew` is installed and run: `brew install cmake m4 automake peg libtool autoconf`.
    * The version of Clang that ships with the OS X toolchain does not support OpenMP, which is a requirement for RedisGraph. One way to resolve this is to run `brew install gcc g++` and follow the on-screen instructions to update the symbolic links. Note that this is a system-wide change - setting the environment variables for `CC` and `CXX` will work if that is not an option.

To build, run `make` in the project's directory.

Congratulations! You can find the compiled binary at: `src/redisgraph.so`

## Installing RedisGraph

RedisGraph is part of [Redis Stack](/docs/stack). See the [Redis Stack download page](/download) for installation options.

## Using RedisGraph

Before using RedisGraph, you should familiarize yourself with its commands and syntax as detailed in the
[command reference](/redisgraph/commands).

You can call RedisGraph's commands from any Redis client.

### With `redis-cli`

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY social "CREATE (:person {name: 'roi', age: 33, gender: 'male', status: 'married'})"
```

### With any other client

You can interact with RedisGraph using your client's ability to send raw Redis commands.
The exact method for doing that depends on your client of choice.

#### Python example

This code snippet shows how to use RedisGraph with raw Redis commands from Python using
[redis-py](https://github.com/redis/redis-py):

```python
import redis

r = redis.Redis()
reply = r.graph("social").query("MATCH (r:Rider)-[:rides]->(t:Team {name:'Ducati'}) RETURN count(r)")
```

## Client libraries

Language-specific clients have been written by the community and the RedisGraph team for 6 languages.

The full list and links can be found on the [Clients](/docs/clients) page.

## Data import

The RedisGraph team maintains the [redisgraph-bulk-loader](https://github.com/redisgraph/redisgraph-bulk-loader) for importing new graphs from CSV files.

The data format used by this tool is described in the [GRAPH.BULK implementation details](/docs/stack/graph/design/bulk_spec).

## Mailing List / Forum

Got questions? Feel free to ask at the [RedisGraph forum](https://forum.redis.com/c/modules/redisgraph).

## License

RedisGraph is licensed under the [Redis Source Available License 2.0 (RSALv2) or the Server Side Public License v1 (SSPLv1)](https://redis.com/legal/licenses/).

<img src="images/logo.svg" alt="logo" width="200"/>

# RedisGraph
[![Forum](https://img.shields.io/badge/Forum-RedisGraph-blue)](https://forum.redislabs.com/c/modules/redisgraph)
[![Discord](https://img.shields.io/discord/697882427875393627?style=flat-square)](https://discord.gg/gWBRT6P)

RedisGraph is the first queryable [Property Graph](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc) database to use [sparse matrices](https://en.wikipedia.org/wiki/Sparse_matrix) to represent the [adjacency matrix](https://en.wikipedia.org/wiki/Adjacency_matrix) in graphs and [linear algebra](http://faculty.cse.tamu.edu/davis/GraphBLAS.html) to query the graph.

Primary features:

* Based on the [Property Graph Model](https://github.com/opencypher/openCypher/blob/master/docs/property-graph-model.adoc)
  * Nodes (vertices) and Relationships (edges) that may have attributes
  * Nodes that can be labeled
  * Relationships have a relationship type
* Graphs represented as sparse adjacency matrices
* [Cypher](http://www.opencypher.org/) as query language
  * Cypher queries translated into linear algebra expressions

To see RedisGraph in action, visit [Demos](https://github.com/RedisGraph/RedisGraph/tree/master/demo).

## Trusted By

<a href="https://www.ibm.com"><img src="images/logo-ibm.jpg" alt="IBM" height="100"/></a>
<a href="https://redislabs.com/case-studies/thinkdata-works/"><img src="images/logo-thinkdata.jpg" alt="thinkdata" height="100"/></a> 
<a href="https://sidy.fr/"><img src="images/logo-sidy.jpg" alt="sidy" height="100"/></a>
<a href="https://www.youtube.com/watch?v=6FYYn-9fPXE"><img src="images/logo-mdmetrix.jpg" alt="mdmetrix" height="100"/></a> 
<a href="https://linkirous.us/"><img src="images/logo-linkurious.jpg" alt="linkirous" height="100"/></a>
<a href="https://medium.com/@tomzeppenfeldt/graphileon-and-redis-labs-become-partners-35bafe6790fa"><img src="images/logo-graphileon.jpg" alt="graphileon" height="100"/></a>
<a href="https://redislabs.com/blog/graphxr-read-modify-and-write-ontologies-with-redisgraph/"><img src="images/logo-kineviz-graphxr.jpg" alt="kineviz" height="100"/></a>


## Quickstart

1. [Redis Cloud](#redis-cloud)
1. [Docker](#docker)
1. [Build](#building)
1. [Start](#loading-redisgraph-into-redis)
1. [Use from any client](#using-redisgraph)

## Redis Cloud

RedisGraph is available on all Redis Cloud managed services.  Redis Cloud Essentials offers a completely free managed database up to 30MB.

[Get started here](https://redislabs.com/try-free/)

## Docker

To quickly try out RedisGraph, launch an instance using docker:

```
docker run -p 6379:6379 -it --rm redislabs/redisgraph
```

### Give it a try

After you load RedisGraph, you can interact with it using redis-cli.

Here we'll quickly create a small graph representing a subset of motorcycle riders and teams 
taking part in the MotoGP championship. Once created, we'll start querying our data.

### With `redis-cli`

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY MotoGP "CREATE (:Rider {name:'Valentino Rossi'})-[:rides]->(:Team {name:'Yamaha'}), (:Rider {name:'Dani Pedrosa'})-[:rides]->(:Team {name:'Honda'}), (:Rider {name:'Andrea Dovizioso'})-[:rides]->(:Team {name:'Ducati'})"
1) 1) Labels added: 2
   2) Nodes created: 6
   3) Properties set: 6
   4) Relationships created: 3
   5) "Query internal execution time: 0.399000 milliseconds"
```

Now that our MotoGP graph is created, we can start asking questions. For example:
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

## Download

A pre-compiled version can be downloaded from [RedisLabs download center](https://redislabs.com/download-center/modules/).

## Building

Requirements:

* The RedisGraph repository: `git clone --recurse-submodules -j8 https://github.com/RedisGraph/RedisGraph.git`

* On Ubuntu Linux, run: `apt-get install build-essential cmake m4 automake peg libtool autoconf`

* On OS X, verify that `homebrew` is installed and run: `brew install cmake m4 automake peg libtool autoconf`.
    * The version of Clang that ships with the OS X toolchain does not support OpenMP, which is a requirement for RedisGraph. One way to resolve this is to run `brew install gcc g++` and follow the on-screen instructions to update the symbolic links. Note that this is a system-wide change - setting the environment variables for `CC` and `CXX` will work if that is not an option.

To build, run `make` in the project's directory.

Congratulations! You can find the compiled binary at: `src/redisgraph.so`

## Loading RedisGraph into Redis

RedisGraph is hosted by [Redis](https://redis.io), so you'll first have to load it as a Module to a Redis server running [Redis v4.0 or above](https://redis.io/download).

We recommend having Redis load RedisGraph during startup by adding the following to your redis.conf file:

```
loadmodule /path/to/module/src/redisgraph.so
```

In the line above, replace `/path/to/module/src/redisgraph.so` with the actual path to RedisGraph's library.

Alternatively, you can have Redis load RedisGraph using this command-line argument syntax:

```sh
$ redis-server --loadmodule /path/to/module/src/redisgraph.so
```

You can also use the [`MODULE LOAD`](http://redis.io/commands/module-load) command. Note, however, that `MODULE LOAD` is a dangerous command and may be blocked/deprecated in the future due to security considerations.

After you've successfully loaded RedisGraph, your Redis log should contain entries similar to:

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

Before using RedisGraph, you should familiarize yourself with its commands and syntax as detailed in the
[commands reference](commands.md).

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
[redis-py](https://github.com/andymccurdy/redis-py):

```python
import redis

r = redis.StrictRedis()
reply = r.execute_command('GRAPH.QUERY', 'social', "CREATE (:person {name:'roi', age:33, gender:'male', status:'married'})")
```

## Client libraries

Language-specific clients have been written by the community and the RedisGraph team for 6 languages.

The full list and links can be found on [the Clients page](clients.md).

## Data import

The RedisGraph team maintains the [redisgraph-bulk-loader](https://github.com/redisgraph/redisgraph-bulk-loader) for importing new graphs from CSV files.

The data format used by this tool is described in the [GRAPH.BULK implementation details](bulk_spec.md).

## Mailing List / Forum

Got questions? Feel free to ask at the [RedisGraph forum](https://forum.redislabs.com/c/modules/redisgraph).

## License

Redis Source Available License Agreement - see [LICENSE](https://raw.githubusercontent.com/RedisGraph/RedisGraph/master/LICENSE)

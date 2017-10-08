# Redis Graph

This project is a Redis module that implements a graph database. Nodes in the graph represent entities such as persons or places, and connections such as 'visit' are made between the different entities.

Suppose we had a person entity representing Barack Obama, it might have two attributes: "age" (55) and "profession" (ex-president). We could also have another entity represent Hawaii with an attribute "population" (1442949). Finally we could construct a simple graph by connecting Barack Obama with an edge representing the relation "born" with Hawaii.

Primary features:

* A graph database implementation
* Nodes and edges may have attributes
* Nodes and edges can be labeled
* Supports [openCypher](http://www.opencypher.org/) graph queries

To see Redis Graph in action see [Demos](https://github.com/RedisLabsModules/redis-module-graph/tree/master/demo).

## Quickstart

1. [Build the Redis Graph module library](#building-the-module)
2. [Load Redis Graph to Redis](#loading-the-module-to-redis)
3. [Use it from any client](#using-redis-graph)

## Building the module

### Linux Ubuntu 16.04

Requirements:

* The Redis Graph repository: `git clone https://github.com/RedisLabsModules/redis-module-graph.git`
* The build-essential package: `apt-get install build-essential`

To build the module, run `make` in the project's directory:

Congratulations! You can find the compiled module library at `src/redisgraph.so`.

## Loading the module to Redis

Requirements:

* [Redis v4.0 or above](https://redis.io/download)

We recommend you have Redis load the module during startup by adding the following to your redis.conf file:

```
loadmodule /path/to/module/redisgraph.so
```

In the line above replace `/path/to/module/redisgraph.so` with the actual path to the module's library. Alternatively, you can have Redis load the module using the following command line argument syntax:

```sh
~/$ redis-server --loadmodule /path/to/module/redisgraph.so
```

Lastly, you can also use the [`MODULE LOAD`](http://redis.io/commands/module-load) command. Note, however, that `MODULE LOAD` is a dangerous command and may be blocked/deprecated in the future due to security considerations.

Once the module has been loaded successfully, the Redis log should have lines similar to:

```
...
30707:M 20 Jun 02:08:12.314 * Module 'graph' loaded from <redacted>/src/redisgraph.so
...
```

## Using Redis Graph

Before using Redis Graph, you should familiarize yourself with its commands and syntax as detailed in the
[commands reference](commands.md) document.

You can call Redis Graph's commands from any Redis client.

### With `redis-cli`

```sh
$ redis-cli
127.0.0.1:6379> GRAPH.QUERY social "CREATE (:person {name: roi, age: 32, gender: male, status: married})"
```

### With any other client

You can call the module's API using your client's ability to send raw Redis commands. Depending on your client of
choice, the exact method for doing that may vary.

#### Python example

This code snippet shows how to use Redis Graph with raw Redis commands from Python with
[redis-py](https://github.com/andymccurdy/redis-py):

```python
import redis

r = redis.StrictRedis()
reply = r.execute_command('GRAPH.QUERY', 'social', 'CREATE (:person {name:roi, age:32, gender:male, status:married)')
```

### Client libraries

Some languages have client libraries that provide support for Redis Graph's commands:

| Project | Language | License | Author | URL |
| ------- | -------- | ------- | ------ | --- |
| redisgraph-py | Python | BSD | [Redis Labs](https://redislabs.com) | [GitHub](https://github.com/swilly22/redisgraph-py) |

# Redis Graph

This project is a Redis module that implements a graph database. Nodes in the graph represent entities such as persons
or places, and connections such as 'visit' are made between the different entities.

Suppose we had a person entity representing Barack Obama, it might have two attributes: "age" (55) and "profession"
(ex-president). We could also have another entity represent Hawaii with an attribute "population" (1442949).
Finally we could construct a simple graph by connecting Barack Obama with an edge representing the relation "born" with
Hawaii.

Primary features:

* A graph database implementation
* Nodes and edges may have attributes
* Nodes and edges can be labeled
* Supports [openCypher](http://www.opencypher.org/) graph queries

To see Redis Graph in action see [Demos](https://github.com/RedisLabsModules/redis-module-graph/tree/master/demo).

## Quickstart

1. [Build the Redis Graph module library](http://redisgraph.io/#building-the-module)
2. [Load Redis Graph to Redis](http://redisgraph.io/#loading-the-module-to-redis)
3. [Use it from any client](http://redisgraph.io/#using-redis-graph)

## Documentation

Read the docs at [redisgraph.io](http://redisgraph.io).

## License

AGPLv3 - see [LICENSE](LICENSE)
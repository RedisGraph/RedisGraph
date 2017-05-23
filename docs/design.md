# RedisGraph: A High Performance In-Memory Graph Database as a Redis Module


## Abstract

Graph-based data is everywhere these days. Facebook, Google, Twitter and Pinterest are just a few of the companies that have realized the power behind relationship data and are utilizing it to the fullest. As a direct result, there's been a rise both in interest for and variety of graph data solutions.

With the introduction of [Redis Modules](http://antirez.com/news/106), we've seen the great potential of introducing a graph data structure to the Redis arsenal.
[RedisGraph](http://redismodules.com/modules/redis-graph/) is a native C implementation with an emphasis on performance. It was developed to bring new graph database capabilities to Redis, and is now available as an open source project on [GitHub](https://github.com/swilly22/redis-module-graph).

In this document, we'll discuss the internal design and features of RedisGraph and demonstrate its current capabilities.


## RedisGraph At-a-Glance

RedisGraph is a graph database developed from scratch on top of Redis, using the new Redis Modules API to extend Redis with new commands and capabilities. Its main features include:
- Simple, fast indexing and querying
- Data stored in RAM, using memory-efficient custom data structures
- On-disk persistence
- Tabular result sets
- Simple and popular query language (Cypher)
- Data filtering, aggregation and ordering


## A Little Taste: RedisGraph in Action
Let’s look at some of the key concepts of RedisGraph using an example over the redis-cli tool:

### Introducing our entities:
It is a common approach to represent entities as nodes within a graph, and RedisGraph follows this simple concept by utilizing Redis' native hash datatype to store its entities.

In this example, we'll create a small graph with both actors and movies as its entities. An "act" relation will connect actors to movies they performed in.

We use the HMSET command to create a new entity:
```sh
HMSET <entity_id> <attribute_name> <attribute_value> <attribute_name> <attribute_value> ...
```

Or in our example:
```sh
HMSET Aldis_Hodge name "Aldis Hodge" birth_year 1986
HMSET O'Shea_Jackson name "O'Shea Jackson" birth_year 1991
HMSET Corey_Hawkins name "Corey Hawkins" birth_year 1988
HMSET Neil_Brown name "Neil Brown" birthyear 1980
HMSET Straight_Outta_Compton title "Straight Outta Compton" genre Biography votes 127258 rating 7.9 year 2015
HMSET Never_Go_Back title "Never Go Back" genre Action votes 15821 rating 6.4 year 2016
```

### Connecting entities:
It is now time to form relationships between actors and movies. We use the RedisGraph ADDEDGE command and specify the source entity, type of connection and destination entity as such:

```sh
GRAPH.ADDEDGE <graph_id> <src_entity_id> <relation> <dest_entity_id>
```

Here we add the Straight Outta Compton cast:

```sh
GRAPH.ADDEDGE movies Aldis_Hodge act Straight_Outta_Compton
GRAPH.ADDEDGE movies O'Shea_Jackson act Straight_Outta_Compton
GRAPH.ADDEDGE movies Corey_Hawkins act Straight_Outta_Compton
GRAPH.ADDEDGE movies Neil_Brown act Straight_Outta_Compton
```

Here we add the only cast member who also played in the movie Never Go Back:

```sh
GRAPH.ADDEDGE movies Aldis_Hodge act Never_Go_Back
```

### Querying the graph:
RedisGraph exposes a subset of Neo4J Cypher language. Although only some of its language capabilities are supported, there's enough functionality to extract valuable insights from your graphs. To execute a query we use the GRAPH.QUERY command:

```sh
GRAPH.QUERY <graph_id> <query>
```

Let's execute a number of queries against our movies graph:

Find the sum, max, min and avg age of the Straight Outta Compton cast:

```sh
GRAPH.QUERY movies "MATCH (actor)-[act]->(movie:"Straight_Outta_Compton") RETURN movie.title, SUM(actor.age), MAX(actor.age), MIN(actor.age), AVG(actor.age)"
```

RedisGraph will reply with:

```sh
1) "Straight Outta Compton,123.000000,37.000000,26.000000,30.750000"
2) "Query internal execution time: 0.108000 milliseconds"
```

The first row contains our query result-set while the second row contains RedisGraph execution time.

Let's try a query to find in how many movies each actor played:

```sh
GRAPH.QUERY movies "MATCH (actor)-[act]->(movie) RETURN actor.name, COUNT(movie.title) AS movies_count ORDER BY movies_count DESC"

1) "Aldis_Hodge,2.000000"
2) "O'Shea Jackson,1.000000"
3) "Corey Hawkins,1.000000"
4) "Neil Brown,1.000000"
5) "Query internal execution time: 0.071000 milliseconds"
```

## The Theory: Ideas behind RedisGraph

Different graph databases uses different structures for representing a graph. Some use an adjacency list, while others might use an adjacency matrix, each with its advantages and disadvantages. For RedisGraph, it was crucial to find a data structure that would enable us to perform fast searches on the graph, so we're using a concept called Hexastore to hold all relationships within a graph.

### Graph representation: Hexastore

A Hexastore is simply a list of triplets, where each triplet is composed of three parts:

1. Subject
2. Predicate
3. Object

The Subject refers to a source node, Predicate represents a relationship and the Object refers to a destination node.
For each relationship within a graph, our Hexastore will contain all six permutations of the source node, relationship edge and destination node. For example, consider the following relation:

`(Aldis_Hodge)-[act]->(Straight_Outta_Compton)`

Aldis_Hodge is the source node,
act is the relationship
and Straight_Outta_Compton is the destination node.

All six possibilities of representing this connection are as follows:

```
SPO:Aldis_Hodge:act:Straight_Outta_Compton
SOP:Aldis_Hodge:Straight_Outta_Compton:act
POS:act:Straight_Outta_Compton:Aldis_Hodge
PSO:act:Aldis_Hodge:Straight_Outta_Compton
OPS:Straight_Outta_Compton:act:Aldis_Hodge
OSP:Straight_Outta_Compton:Aldis_Hodge:act
```

With the Hexastore constructed we can easily search our graph. Suppose I would like to find the full cast of the movie Straight Outta Compton, all I need to do is search my Hexastore for all strings containing the prefix: `OPS:Straight_Outta_Compton:act:*`

Or if I'm interested in all the movies Aldis Hodge played in, I can search for all strings containing the prefix: `SPO:Aldis_Hodge:act:*`

Although a Hexastore uses plenty of memory (six triplets for each relation), we're using a trie data structure, which is not only fast in terms of search but is also memory efficient as it doesn't duplicate string prefixes it has already seen.

### The Design: Departing From Redis Data Structures

At first, we stored the Hexastore within a Redis native sorted-set data structure, but with the ability to introduce new data structures to Redis, we've switched to a trie. This allows for more efficient search time and reduces overall memory consumption.

As mentioned earlier, nodes within the RedisGraph are no more than a reference to a Redis hash object that hold entities and attributes. By using Redis Hashes, we gain a number of nice features:
- Fast insertion (Redis is known for its speed, and creating hash objects / entities is a quick operation to perform).
- Simple reuse of entities already stored in Redis (there's no need to reintroduce / duplicate them).
- Data reusability (other Redis users which might not be aware that RedisGraph can use these objects, so RedisGraph enjoys "free" updates to its entities).


### Query language: Cypher
There are a number of Graph Query languages, and we didn't want to reinvent the wheel. We've decided to implement a subset of one of the most popular graph query languages out there, called Cypher by Neo4J. Allthough the Open-Cypher project allows us to create a parser for the language, we decided to build our own parser with Lex as a tokenizer and Lemon to generate a C target parser.

As mentioned, only a subset of the language is supported, but it is our intention to continue adding new capabilities and extend the language.

## Runtime: query execution
Let's review the steps our module takes when executing a query.
Consider the following query, which finds all actors who've played alongside Aldis Hodge and are over 30 years old:
```
MATCH (:Aldis_Hodge)-[act]->(Movie)<-[act]-(Actor) WHERE Actor.age > 30 RETURN Movie.title, Actor.name
```

RedisGraph will:
- Parse the query, and build an abstract syntax tree (AST)
- Build a graph representation from the MATCH clause
- Construct a filter tree
- Search for matching entities
- Populate result-set with matching entities attributes

### Query parser
Given a valid query, the parser will generate an AST containing four primary nodes (one for each clause):

1. MATCH
2. WHERE
3. RETURN
4. ORDER

Generating an abstract syntax tree is a common way of describing and structuring a language.

### Filter tree
A query can filter out entities by creating predicates. In our example we're filtering actors who are younger then 30.
It's possible to combine predicates using the OR, AND keywords to form granular conditions. During runtime, the WHERE clause is used
to construct a filter tree, and each node within the tree is either a condition e.g. A > B or an operation (AND/OR). Candidate entities are evaluated as they pass through the tree.


### Query processing
The MATCH clause describes relations between queried entities (nodes). A node can have an alias, which will allow us to refer to it
at later stages within the executing query lifetime (WHERE, RETURN clause). However, all nodes must eventually be assigned an ID -
this process is referred to as the search phase.

During the search, we'll be querying the Hexastore for IDs according to the MATCH clause structure.
For instance, in our example we'll start our search by looking for movies in which Aldis Hodge played.
For each movie, we'll extend our search to find out which other actors appeared.

It makes sense to start searching from a node that already has an ID
assigned to it, as this will reduce our search space dramatically.
As you might imagine, the search process is a recursive operation that traverses the graph. At each step a new ID is 
discovered, and once every node has an ID assigned we can apply our filter tree to the graph. If it passes all filters,
we'll simply extract the requested attributes (as specified in the return clause) and append a new record to the final result set.

## Benchmarks:

Depending on the underlying hardware results may vary. That said, inserting a new relationship is done in O(1). RedisGraph is able to create 100K new relations within one second.

Retrieving data really depends on the size of the graph and the type of query you're executing. On a small size graph (~1000 entities and ~2500 edges), RedisGraph is able to perform ~30K friend of a friend queries every second.

It's worth mentioning that besides the Hexastore, entities are not indexed. It’s our intention to introduce entitiy indexing, which should decrease query execution time dramatically.


## License:
RedisGraph is published under AGPL-3.0.

## Conclusion:
RedisGraph, although still a young project, can be an alternative to other graph databases, with a subset of operations that you can use it to analyze and explore graph data. Being a Redis module, this project is accessible from every Redis client without requiring  adjustments. It's our intention to keep on improving and extending RedisGraph with the help of the open source community.

# RedisGraph: A High Performance In-Memory Graph Database

## Abstract

Graph-based data is everywhere nowadays. Facebook, Google, Twitter and Pinterest are just a few who've realize the power
behind relationship data and are utilizing it to its fullest. As a direct result, we see a rise both in interest for and 
the variety of graph data solutions.

With the introduction of [Redis Modules](http://antirez.com/news/106) we've recognized the great potential of introducing a
graph data structure to the Redis arsenal, and developed RedisGraph. Bringing new graph capabilities to Redis 
through a native C implementation with an emphasis on performance, [RedisGraph](https://github.com/RedisGraph/RedisGraph) is now
available as an open source project.

In this documentation, we'll discuss the internal design and features of RedisGraph and demonstrate its current capabilities.

## RedisGraph At-a-Glance

RedisGraph is a graph database developed from scratch on top of Redis, using the new Redis Modules API to extend Redis
with new commands and capabilities. Its main features include:

- Simple, fast indexing and querying
- Data stored in RAM using memory-efficient custom data structures
- On-disk persistence
- Tabular result sets
- Uses the popular graph query language [openCypher](https://neo4j.com/docs/developer-manual/3.4/cypher/)

## A Little Taste: RedisGraph in Action

Let’s look at some of the key concepts of RedisGraph using this example over the redis-cli tool:

### Constructing a graph

It is common to represent entities as nodes within a graph. In this example, 
we'll create a small graph with both actors and movies as its entities, 
and an "act" relation that will connect actors to the movies they acted in.
We'll use the graph.QUERY command to issue a CREATE query, 
which will introduce new entities and relations to our graph.

```sh
graph.QUERY <graph_id> 'CREATE (:<label> {<attribute_name>:<attribute_value>,...})'
```

```sh
graph.QUERY <graph_id> 'CREATE (<source_node_alias>)-[<relation> {<attribute_name>:<attribute_value>,...}]->(<dest_node_alias>)'
```

We'll construct our graph in one command:

```sh
graph.QUERY IMDB 'CREATE (aldis:actor {name: "Aldis Hodge", birth_year: 1986}),
                         (oshea:actor {name: "OShea Jackson", birth_year: 1991}),
                         (corey:actor {name: "Corey Hawkins", birth_year: 1988}),
                         (neil:actor {name: "Neil Brown", birth_year: 1980}),
                         (compton:movie {title: "Straight Outta Compton", genre: "Biography", votes: 127258, rating: 7.9, year: 2015}),
                         (neveregoback:movie {title: "Never Go Back", genre: "Action", votes: 15821, rating: 6.4, year: 2016}),
                         (aldis)-[:act]->(neveregoback),
                         (aldis)-[:act]->(compton),
                         (oshea)-[:act]->(compton),
                         (corey)-[:act]->(compton),
                         (neil)-[:act]->(compton)'
```

### Querying the graph

RedisGraph exposes a subset of the openCypher graph language. Although only some language capabilities are supported,
there's enough functionality to extract valuable insights from your graphs. To execute a query, we'll use the GRAPH.QUERY
command:

```sh
GRAPH.QUERY <graph_id> <query>
```

Let's execute a number of queries against our movies graph.

Find the sum, max, min and avg age of the 'Straight Outta Compton' cast:

```sh
GRAPH.QUERY IMDB 'MATCH (a:actor)-[:act]->(m:movie {title:"Straight Outta Compton"})
RETURN m.title, SUM(2020-a.birth_year), MAX(2020-a.birth_year), MIN(2020-a.birth_year), AVG(2020-a.birth_year)'
```

RedisGraph will reply with:

```sh
1) 1) "m.title"
   2) "SUM(2020-a.birth_year)"
   3) "MAX(2020-a.birth_year)"
   4) "MIN(2020-a.birth_year)"
   5) "AVG(2020-a.birth_year)"
2) 1) 1) "Straight Outta Compton"
      2) "135"
      3) (integer) 40
      4) (integer) 29
      5) "33.75"
```

* The first row is our result-set header which names each column according to the return clause.
* The second row contains our query result.

Let's try another query. This time, we'll find out how many movies each actor played in.

```sh
GRAPH.QUERY IMDB "MATCH (actor)-[:act]->(movie) RETURN actor.name, COUNT(movie.title) AS movies_count ORDER BY
movies_count DESC"

1) "actor.name, movies_count"
2) "Aldis Hodge,2.000000"
3) "O'Shea Jackson,1.000000"
4) "Corey Hawkins,1.000000"
5) "Neil Brown,1.000000"
```

## The Theory: Ideas behind RedisGraph

### Representation

RedisGraph uses sparse adjacency matrices to represent graphs. As directed relationship connecting source node S to destination node T is
recorded within an adjacency matrix M, by setting M's S,T entry to 1 (M[S,T]=1).
<!-- The above sentence is not clear -->
As a rule of thumb, matrix rows represent source nodes while matrix columns represent destination nodes.

Every graph stored within RedisGraph has at least one matrix, referred to as THE adjacency matrix (relation-type agnostic). In addition, every relation with a type has its own dedicated matrix. Consider a graph with two relationships types:
<!-- What is the meaning of THE? -->

1. visits
2. friend

The underlying graph data structure maintains three matrices:

1. THE adjacency matrix - marking every connection within the graph
2. visit matrix - marking visit connections
3. friend matrix - marking friend connections

A 'visit' relationship E that connects node A to node B, sets THE adjacency matrix at position [A,B] to 1. Also, the visit matrix V sets position V[A,B] to 1.

To accommodate typed nodes, one additional matrix is allocated per label, and a label matrix is symmetric with ones along the main diagonal. Assume that node N was labeled as a Person, then the Person matrix P sets position P[N,N] to 1.

This design lets RedisGraph modify its graph easily, including:

- Adding new nodes simply extends matrices, adding additional rows and columns
- Adding new relationships by setting the relevant entries at the relevant matrices
- Removing relationships clears relevant entries
- Deleting nodes by deleting matrix row/column.

One of the main reasons we chose to represent our graphs as sparse matrices is graph traversal.

### Traversal

Graph traversal is done by matrix multiplication. For example, if we wanted to find friends of friends for every node in the graph, 
this traversal can be expressed by FOF = F^2. F stands for the friendship relation matrix, and the result matrix FOF summarizes the traversal.
The rows of the FOF represent source nodes and its columns represent friends who are two hops away: if FOF[i,j] = 1 then j is a friend of a friend of i.

### Algebraic expression

When a search pattern such as `(N0)-[A]->(N1)-[B]->(N2)<-[A]-(N3)` is used as part of a query, we translate it into a set of matrix multiplications. For the given example, one possible expression would be: `A * B * Transpose(A)`.

Note that matrix multiplication is an associative and distributive operation. This gives us the freedom to choose which terms we want to multiply first (preferring terms that will produce highly sparse intermediate matrices). It also enables concurrency when evaluating an expression, e.g. we could compute A*Z and B*Y in parallel.

### GraphBLAS

To perform all of these operations for sparse matrices, RedisGraph uses [GraphBLAS](http://graphblas.org/) - a standard API similar to BLAS. The current implementation uses the CSC sparse matrix format (compressed sparse columns), although the underlying format is subject to change.

## Query language: openCypher

There are a number of graph query languages, so we didn't want to reinvent the wheel. We decided to implement a subset of one of the most popular graph query languages out there: openCypher
While the openCypher project provides a parser for the language, we decided to create our own parser. We used Lex as a tokenizer and Lemon to generate a C target parser.

As mentioned earlier, only a subset of the language is currently supported, but we plan to continue adding new capabilities
and extend RedisGraph's openCypher capabilities.

## Runtime: query execution

Let's review the steps RedisGraph takes when executing a query.

Consider this query that finds all actors who played alongside Aldis Hodge and are over 30 years old:

```sh
MATCH (aldis::actor {name:"Aldis Hodge"})-[:act]->(m:movie)<-[:act]-(a:actor) WHERE a.age > 30 RETURN m.title, a.name
```

RedisGraph will:

- Parse the query, and build an abstract syntax tree (AST)
- Compose traversal algebraic expressions
- Build filter trees
- Construct an optimized query execution plan composed of:
    - Filtered traverse
    - Conditional traverse
    - Filter
    - Project
- Execute the plan
- Populate a result set with matching entity attributes

### Filter tree

A query can filter out entities by creating predicates. In our example, we filter actors younger then 30.

It's possible to combine predicates using OR and AND keywords to form granular conditions.

During runtime, the WHERE clause is used to construct a filter tree, and each node within the tree is either a condition (e.g. A > B) or an operation (AND/OR). When finding candidate entities, they are passed through the tree and evaluated.

## Benchmarks

Depending on the underlying hardware results may vary. However, inserting a new relationship is done in O(1).
RedisGraph is able to create over 1 million nodes under half a second and form 500K relations within 0.3 of a second.

## License

RedisGraph is published under Redis Source Available License Agreement.

## Conclusion

Although RedisGraph is still a young project, it can be an alternative to other graph databases. With its subset of
operations, you can use it to analyze and explore graph data. Being a Redis Module, this project is accessible from
every Redis client without adjustments. It's our intention to keep on improving and extending
RedisGraph with the help of the open source community.

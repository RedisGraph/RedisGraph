# RedisGraph: A High Performance In-Memory Graph Database

## Abstract

Graph based data is everywhere now days, Facebook, Google, Twitter and Pinterest are only a few who've realize the power
behind relationship data and are utilizing it to the fullest, as a direct result we see a rise both in interest and
variety of graph data solutions.

With the introduction of [Redis Modules](http://antirez.com/news/106) we've seen the great potential of introducing a
graph data structure to Redis arsenal, a native C implementation with emphasis on performance was developed to bring
new graph capabilities to Redis, the [RedisGraph](https://github.com/RedisLabsModules/redis-graph) is now
available as an open source project.

In this document we'll discuss the internal design and feature of RedisGraph and demonstrate its current capabilities.


## RedisGraph At-a-Glance

RedisGraph is a graph database developed from scratch on top of Redis, using the new Redis Modules API to extend Redis
with new commands and capabilities. Its main features include:

- Simple, fast indexing and querying
- Data stored in RAM, using memory-efficient custom data structures
- On disk persistence
- Tabular result sets
- Simple and popular graph query language [Cypher](https://neo4j.com/docs/developer-manual/3.4/cypher/)

## A Little Taste: RedisGraph in Action

Let’s look at some of the key concepts of RedisGraph using this example over the redis-cli tool:

### Constructing a graph

It is a common concept to represent entities as nodes within a graph, In this example, we'll create a small graph with both actors and movies as its entities,
an "act" relation will connect actors to movies they casted in.
We use the graph.QUERY command to issue a CREATE query which will introduce new entities and relations to our graph.

```sh
graph.QUERY <graph_id> 'CREATE (:<label> {<attribute_name>:<attribute_value>,...})'
```

```sh
graph.QUERY <graph_id> 'CREATE (<source_node_alias>)-[<relation> {<attribute_name>:<attribute_value>,...}]->(<dest_node_alias>)'
```

Construct our graph in one go:

```sh
graph.QUERY IMDB 'CREATE (aldis:actor {name: "Aldis Hodge", birth_year: 1986}),
                         (oshea:actor {name: "OShea Jackson", birth_year: 1991}),
                         (corey:actor {name: "Corey Hawkins", birth_year: 1988}),
                         (neil:actor {name: "Neil Brown", birthyear: 1980}),
                         (compton:movie {title: "Straight Outta Compton", genre: "Biography", votes: 127258, rating: 7.9, year: 2015}),
                         (neveregoback:movie {title: "Never Go Back", genre: "Action", votes: 15821, rating: 6.4, year: 2016}),
                         (aldis)-[act]->(neveregoback),
                         (aldis)-[act]->(compton),
                         (oshea)-[act]->(compton),
                         (corey)-[act]->(compton),
                         (neil)-[act]->(compton)'
```

### Querying the graph

RedisGraph exposes a subset of openCypher graph language, although only a number of language capabilities are supported
there's enough functionality to extract valuable insights from your graphs, to execute a query we use the GRAPH.QUERY
command:

```sh
GRAPH.QUERY <graph_id> <query>
```

Let's execute a number of queries against our movies graph:

Find the sum, max, min and avg age of the Straight Outta Compton cast:

```sh
GRAPH.QUERY IMDB "MATCH (a:actor)-[act]->(m:movie {title:\"Straight Outta Compton\"})
RETURN m.title, SUM(a.age), MAX(a.age), MIN(a.age), AVG(a.age)"
```

RedisGraph will reply with:

```sh
1) "m.title, SUM(a.age), MAX(a.age), MIN(a.age), AVG(a.age)"
2) "Straight Outta Compton,123.000000,37.000000,26.000000,30.750000"
```

The first row is our result-set header which name each column according to the return clause.
Second row contains our query result.

Let's try another query, this time we'll find in how many movies each actor played.

```sh
GRAPH.QUERY IMDB "MATCH (actor)-[act]->(movie) RETURN actor.name, COUNT(movie.title) AS movies_count ORDER BY
movies_count DESC"

1) "actor.name, movies_count"
2) "Aldis Hodge,2.000000"
3) "O'Shea Jackson,1.000000"
4) "Corey Hawkins,1.000000"
5) "Neil Brown,1.000000"
```

## The Theory: Ideas behind RedisGraph

### Representation

RedisGraph uses sparse adjacency matrices to represent graphs, a directed edge connecting source node S to destination node T is
recorded within an adjacency matrix M, by setting M's S,T entry to 1, M[S,T]=1.
As a rule of thumb matrix rows represent source nodes while matrix columns represent destination nodes.

Every graph stored within RedisGraph has at least one matrix, refereed to as THE adjacency matrix (relation type agnostic), in addition every typed relation has its own dedicated typed matrix, consider a graph with two types of edges:

1. visits
2. friend

The underline graph data structure would maintain three matrices:

1. THE adjacency matrix - marking every connection within the graph
2. visit matrix - marking visit connections
3. friend matrix - marking friend connections

A visit edge E connecting node A to node B would set THE adjacency matrix at position [A,B] to 1 in addition the visit matrix V would also be set at position V[A,B] to 1.

To accommodate typed nodes additional matrices are allocated one per label, a label matrix is promised to be symmetric with ones along the main diagonal, assume node N was labeled as a Person, then the Person matrix P would be set at position P[N,N] to 1.

This design enables RedisGraph to modify its graph with ease:

- Adding new nodes simply means extending matrices, adding additional rows and columns.
- Adding new edges is done by setting the relevant entries at the relevant matrices.
- Removing edges means clearing relevant entires.
- Deleting nodes is achieved by matrix row/column deletion.

One of the main reason why we've chosen to represent our graphs as sparse matrices has to do with graph traversal.

### Traversal

Graph traversal is accomplished by matrix multiplication, suppose we wanted to find friends of friends for every node in the graph,
this traversal can be expressed by FOF = F^2, where F stands for the friendship relation matrix, the result matrix FOF summarizes the traversal,
its rows represent source nodes while its columns represent friends who are 2 hops away, if FOF[i,j] = 1 then j is a friend of a friend of i.

### Algebraic expression

When a search pattern such as `(N0)-[A]->(N1)-[B]->(N2)<-[A]-(N3)` is given as part of a query we translate it into a set of matrix multiplications, for the given example, one possible expression would be: `A * B * Transpose(A)`.

We should note that matrix multiplication is an Associative and Distributive operation, this gives us the freedom in choosing which terms we would like to multiply first (preferring terms which will produce highly sparse intermediate matrices) and enables concurrency when evaluating an expression: we could compute A*Z and B*Y in parallel.

### GraphBLAS

To perform all of its sparse matrix operation RedisGraph uses [GraphBLAS](http://graphblas.org/) a standard API similar to BLAS for sparse matrix operations, the current implementation uses the CSC sparse matrix format (compressed sparse columns), the underline format might be a subject to change.

## Query language: openCypher

There are a number of Graph Query languages, we didn't want to reinvent the wheel and come up with our own language,
and so we've decided to implement a subset of one of the most popular graph query language out there openCypher.
The Open-Cypher project provides means to create a parser for the language, although convenient
we decided to create our own parser with Lex as a tokenizer and Lemon which generates a C target parser.

As mentioned only a subset of the language is supported, but it is our intention to continue adding new capabilities
and extend the language.

## Runtime: query execution

Let's review the steps RedisGraph takes when executing a query.
Consider the following query which finds all actors who've played alongside Aldis Hodge and are over 30 years old:

```sh
MATCH (aldis::actor {name:"Aldis Hodge"})-[act]->(m:movie)<-[act]-(a:actor) WHERE a.age > 30 RETURN m.title, a.name
```

RedisGraph will

- Parse query, build abstract syntax tree (AST)
- Compose traversal algebraic expression
- Build filter trees
- Construct an optimized query execution plan composed of:
    - Filtered traverse
    - Conditional traverse
    - Filter
    - Project
- Execute plan
- Populate result-set with matching entities attributes

### Filter tree

A query can filter out entities by creating predicates. In our example we're filtering actors which are younger then 30.

It's possible to combine predicates using the OR, AND keywords to form granular conditions.

During runtime the WHERE
clause is used to construct a filter tree, each node within the tree is either a condition e.g. A > B or an operation
(AND/OR), when finding candidate entities they are passed through the tree and get evaluated.

## Benchmarks

Depending on the underlying hardware results may vary.

That said, inserting a new relationship is done in O(1).
RedisGraph is able to create over 1 million nodes under half a second and form 500K relations within 0.3 of a second.

Retrieving data really depends on the size of the graph and the type of query you're executing,
.........

## License

Redis-Graph is published under Apache 2.0 with Commons Clause.

## Conclusion

Although RedisGraph is still a young project, it can be an alternative to other graph databases. With its subset of
operations one can use it to analyze and explore its graph data. Being a Redis module this project is accessible from
every Redis client without the need to make any adjustments. It's our intention to keep on improving and extending
RedisGraph with the help of the open source community.

# Redis graph


### Introduction

This project is a Redis module that implements a graph database. nodes in the graph represent entities such as persons or places, and connections such as 'visit' are made between the different entities.

Suppose we had a person entity representing Barack Obama, it might have two attributes: "age" (55) and "profession" (ex-president). We could also have another entity represent Hawaii with an attribute "population" (1442949).

Finally we could construct a simple graph by connecting Barak Obama with an edge representing the relation "born" with Hawaii.

### Demo
To see RedisGraph in action see [Demos](https://github.com/swilly22/redis-module-graph/tree/master/Demo)

### Usage

- GRAPH.CREATENODE
- GRAPH.ADDEDGE
- GRAPH.REMOVEEDGE
- GRAPH.DELETE
- GRAPH.EXPLAIN
- GRAPH.QUERY

### GRAPH.CREATENODE

Creats a new node within the given graph, the node is marked with a label (if provided)

Arguments: `Graph name, label [optional], list of key value pairs`

Returns: `Node ID`

```sh
GRAPH.CREATENODE us_government presidents name "Barak Obama" age 55
```

#### GRAPH.ADDEDGE

Creats a connection within the given graph between source node and destination node using relation.

Arguments: `Graph name, source node ID, relationship, destination node ID`

Returns: `Edge ID`

```sh
GRAPH.ADDEDGE us_government Barak_Obama_Node_ID born Hawaii_Node_ID
```

#### GRAPH.REMOVEEDGE

Removes edge connecting source to destination.

Arguments: `Graph name, source node ID, relationship, destination node ID`

Returns: `Null`

```sh
GRAPH.REMOVEEDGE us_government Richard_Nixon_Node_ID born California_Node_ID
```

#### GRAPH.DELETE

Deletes the entire graph

Arguments: `Graph name`

Returns: `Null`

```sh
GRAPH.DELETE us_government
```

#### GRAPH.EXPLAIN

Constructs a Query Execution Plan but does not run it, inspect this execution plan to better
understand how your query get executed.

Arguments: `Graph name, Query`

Returns: `String representation of a query execution plan`

```sh
GRAPH.EXPLAIN us_government "MATCH (p:president)-[born]->(h:state {name:Hawaii}) RETURN p"
```

#### GRAPH.QUERY

Execute the given query against specified graph

Arguments: `Graph name, Query`

Returns: `Result set`

```sh
GRAPH.QUERY us_government "MATCH (p:president)-[born]->(h:state {name:Hawaii}) RETURN p"
```

### Query language

The syntax is based on neo4j's Cypher and currently only a subset of the language is supported.

A query is composed of five parts:

### Query structure

- MATCH
- WHERE
- RETURN
- ORDER BY
- LIMIT

#### MATCH

Describes the relationship between queried entities, it is composed of three parts:

- Source node (S)
- Relationship [R]
- Destination node (D)

Combining the three together
`(S)-[R]->(D)`

Each node can contain an alias and a label, but nodes can be left empty if needed.

Node structure: `(alias:label {filters})` alias, label and filters are all optional.

Example:

```sh
(a:actor)-[act]->(m:movie {title:"straight outta compton"})
```

`a` is an alias for the source node which we'll be able to refer to at different places within our query,

`actor` is the label under which this node is marked

`act` is the relationship

`m` an alias for the destination node

`movie` destination node is of "type" movie

`{title:"straight outta compton"}` requires node's title attribute to equal to "straight outta compton"

As such we're interested in actor entities which have the relation "act" with **the** entity representing the "straight outta compton" movie.

It is possible to describe broder relationships by composing a multi hop query as such:

```sh
(me {name:swilly})-[friends_with]->()-[friends_with]->(fof)
```

Here we're interesed to find out who are my friends friends.

Nodes can have more than one edge coming in or out of them, for instance:

```sh
(me {name:swilly})-[visited]->(c:country)<-[visited]-(friend)<-[friends_with]-({name:swilly})
```

Here we're interested in knowing which of my friends have visited at least one country I've been to.


#### WHERE

This clause is not mandatory, but in order to filter results you can define prediactes of two kinds:

1.Compare against constant value: `alias.property operation value`
where `value` is a primitive type (int, float, string and boolean)

2.Compare between nodes properties: `alias.property operation alias.property`

Supported operations:

- `=`
- `!=`
- `<`
- `<=`
- `>`
- `>=`

Predicates can be combided using AND / OR, wrap predicates within parentheses to control precedence.


Examples:

```sh
WHERE (actor.name = "john doe" OR movie.rating > 8.8) AND movie.votes <=250)
```

```sh
WHERE actor.age >= director.age AND actor.age > 32
```

It is also possible to specify equality predicates within a node using the curly braces as such:

```sh
(president {name:"Barack Obama"})
```

Here we've required that the president node's name will have the value "Barack Obama"
There's no difference between inlined predicates and predicates specified within the WHERE clause.

#### RETURN
In its simple form Return defines which properties the returned result-set will contain,
its structure is a list of `alias.property` seperated by comma,
for convenience it's possible to specify only alias when you're interested in every attribute an entity possess
and don't want to specify each attribute individually. e.g.

```sh 
RETURN movie.title, actor
```

Use the DISTINCT keyword to remove duplications within the result-set

```sh
RETURN DISTINCT friend_of_friend.name
```

In the above example, suppose we have two friends Joe and Miesha
and both know Dominick then DISTINCT will make sure Dominick will only appear once
in the final result-set.


Return can also be used to aggregate data similar to SQL group by, once an aggregation function is added to the return list all other none aggregated values are considered as group keys, for example:

```sh
RETURN movie.title, MAX(actor.age), MIN(actor.age)
```

Here we group data by movie title and foreach movie we find its youngest and oldest actor age.

#### Aggregations
Supported aggregation functions:

- `sum`
- `avg`
- `min`
- `max`
- `count`

### ORDER BY
Specifies that the output should be sorted and how.

You can order by multiple properties by stating each variable in the ORDER BY clause.
the result will be sorted by the first variable listed, and for equals values, go to the next property in the ORDER BY clause, and so on.

```sh
ORDER BY <alias.property list> [ASC/DESC]
```

Below we sort our friends by height, for similar heights, weight is used to break even.

```sh
ORDER BY friend.height, friend.weight DESC
```

### LIMIT

Although not mandatory, but in order to limit the number of records returned by a query
use the limit clause as such

```sh
LIMIT <max records to return>
```

If not specified there's no limit to the number of records returned by a query.

### Build and run
To build the module, from root folder run:

```sh
cmake . && make module
```

Loading module into redis:

```sh
# Assuming you have a redis build from the unstable branch:
/path/to/redis-server --loadmodule <PATH_TO_RedisGraph>/src/libmodule.so
```

# Redis graph


### Introduction
This project is an attempt to build a graph database ontop of Redis using Redis modules, where the nodes in the graph represents entities such as a person or a place and connections such as 'visit' are made between the different entities.

As entities might have several attributes Redis hashes seems ideal to hold them, as such a node in the graph is named after an entity's key, for example suppose we had a persone entity representing Barack Obama, our entity's key within redis would be "Barack Obama" and two of its attributes could be Age (55) and profession, we could also have another entity under the key Hawaii. with an attribute population (1,442,949).
Finally we could construct a simple graph by connecting Barak Obama with an edge representing the relation born to Hawaii.

This is one of the key ideas behind this project; a node in the graph is simply a reference to an entity stored as Redis hash.


### Usage
This module introduce four new commands
- GRAPH.ADDEDGE
- GRAPH.REMOVEEDGE
- GRAPH.DELETE
- GRAPH.QUERY

#### GRAPH.ADDEDGE
Arguments: `Graph name, source node, relationship, destination node`

Creats a connection within the given graph between source node and destination node using relation.
```sh
GRAPH.ADDEDGE presidents "Barak Obama" born Hawaii
```
#### GRAPH.REMOVEEDGE
Arguments: `Graph name, source node, relationship, destination node`

Removes edge connecting source to destination.
```sh
GRAPH.REMOVEEDGE presidents "Richard Nixon" born California
```
#### GRAPH.DELETE
Arguments: `Graph name`

Deletes the entire graph
```sh
GRAPH.DELETE presidents
```

#### GRAPH.QUERY
Arguments: `Graph name, Query`

Execute the given query against specified graph

```sh
GRAPH.QUERY presidents "MATCH (president)-[born]->(state:Hawaii) RETURN president.name, president.age"
```

### Query language
The syntex is based on neo4j's Cypher and currently only a subset of the language is supported.

A query is composed of four parts:

### Query structure

 - MATCH
 - WHERE
 - RETURN
 - LIMIT

#### MATCH
Describes the relationship between queried entities, it is composed of three parts:
- Source node (S)
- Relationship [R]
- Destination node (D)

Combining the three together
`(S)-[R]->(D)`

Each node can contain an alias and an entity ID, but nodes can be left empty if needed.

Node structure: `(alias:id)` alias and id are both optional.

Example:
```sh
(actor)-[act]->(movie:"straight outta compton")
```
`actor` is an alias for the source node which we'll be able to refer to at different places within our query,

`act` is the relationship

As such we're interested in entities which have the relation "act" with **the** entity "straight outta compton"

It is possible to describe broder relationships by composing a multi hop query as such:
```sh
(me:swilly)-[friends_with]->()-[friends_with]->(fof)
```
Here we're interesed to find out who are my friends friends.

Nodes can have more than one edge coming in or out of them, for instance:
```sh
(me:swilly)-[visited]->(country)<-[visited]-(friend)<-[friends_with]-(me)
```
Here we're interested in knowing which of my friends have visited at least one country I've been to.


#### WHERE
This clause is not mandatory, but in order to filter results you can define prediactes of two kinds:
1. Compare against constant value: `alias.property operation value`
where `value` is a primitive type (int, float, string and boolean)

2. Compare between nodes properties: `alias.property operation alias.property`

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

#### RETURN
In its simple form Return defines which properties the returned result-set will contain, its structure is a list of `alias.property` seperated by comma. e.g. 
```sh 
RETURN movie.title, actor.name
```

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

### LIMIT

Although not mandatory, but in order to limit the number of records returned by a query
use the limit clause as such

```sh
LIMIT <max records to return>
```

If not specified there's no limit to the number of records returned by a query.

#### Build and run
To build the module, from root folder run:
```sh 
cmake . && make all
``` 
Loading module into redis:
```sh 
# Assuming you have a redis build from the unstable branch:
/path/to/redis-server --loadmodule <PATH_TO_RedisGraph>/src/libmodule.so
``` 

For more eaxmples please see [Demo](https://github.com/swilly22/redis-module-graph/tree/master/Demo)
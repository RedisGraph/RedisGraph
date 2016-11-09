# Redis graph


### Introduction
This project is an attempt to build a graph database ontop of Redis using Redis modules, where the nodes in the graph represents entities such as a person or a place and connections such as 'visit' are made between the different entities.

As entities might have several attributes Redis hashes seems ideal to hold them, as such a node in the graph is named after an entity's key, for example supose we had a persone entity representing Barack Obama, our entity's key within redis would be "Barack Obama" and two of its attributes could be Age (55) and profession, we could also have another entity under the key Hawaii. with an attribute population (1,442,949).
Finally we could construct a simple graph by connecting Barak Obama with an edge representing the relation born to Hawaii.

This is one of the key ideas behind this project; a node in the graph is simply a reference to an entity stored as Redis hash.


### Usage
This module introduce two new commands
- GRAPH.ADDEDGE
- GRAPH.QUERY

#### ADDEDGE
Arguments: `Graph name, source node, relationship, destination node`

Creats a connection within the given graph between source node and destination node using relation.
```sh
GRAPH.ADDEDGE presidents "barak obama", born Hawaii
```

#### GRAPH.QUERY
Arguments: `Graph name, Query`

Execute the given query against specified graph

```sh
GRAPH.QUERY presidents "MATCH (president)-[born]->(state:Hawaii) RETURN president.name, president.age"
```

### Query language
The syntex is based on neo4j's Cypher and currently only a small subset of the language is supported.

A query is composed of three parts:

### Query structure

 - MATCH
 - WHERE
 - RETURN

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

At the moment the language only suport a single hop relation, in future versions several hops will be possible.

#### WHERE
this clause is not mandatory, but in order to filter results define prediactes as such: `alias.property operation value`

supported operations:
- `=`
- `!=`
- `<`
- `<=`
- `>`
- `>=`

Predicates can be combided using AND / OR, wrap predicates within parentheses to control precedence.

At the moment `value` can only represent a primitive type (int, float, string and boolean), in the future it will be possible to replace primitive values with node's properties.

Example: 
```sh
WHERE (actor.name = "john doe" OR movie.rating > 8.8) AND movie.votes <=250)
```

#### RETURN
Defines which properties the returned result-set will contain, its structure is a list of `alias.property` seperated by comma. e.g. 
```sh 
RETURN movie.title, actor.name
```

#### Build and run
To build the module, from root folder run:
```sh 
cmake . && make
``` 
Loading module into redis:
```sh 
./redis-server --loadmodule <PATH_TO_RedisGraph>/src/libmodule.so
``` 

For more information please see Demo
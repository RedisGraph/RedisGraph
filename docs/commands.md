# Redis Graph Commands

## GRAPH.QUERY

Executes the given query against a specified graph.

Arguments: `Graph name, Query`

Returns: [Result set](result_structure.md#redisgraph-result-set-structure)

```sh
GRAPH.QUERY us_government "MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
```

### Query language

The syntax is based on [Cypher](http://www.opencypher.org/), and only a subset of the language currently
supported.

1. [Clauses](#query-structure)
2. [Functions](#functions)

### Query structure

- MATCH
- WHERE
- RETURN
- ORDER BY
- SKIP
- LIMIT
- CREATE
- MERGE
- DELETE
- SET
- WITH

#### MATCH

Match describes the relationship between queried entities, using ascii art to represent pattern(s) to match against.

Nodes are represented by parenthesis `()`,
and Relationships are represented by brackets `[]`.

Each graph entity node/relationship can contain an alias and a label/relationship type, but both can be left empty if necessary.

Entity structure: `alias:label {filters}`.

Alias, label/relationship type and filters are all optional.

Example:

```sh
(a:actor)-[:act]->(m:movie {title:"straight outta compton"})
```

`a` is an alias for the source node, which we'll be able to refer to at different places within our query.

`actor` is the label under which this node is marked.

`act` is the relationship type.

`m` is an alias for the destination node.

`movie` destination node is of "type" movie.

`{title:"straight outta compton"}` requires the node's title attribute to equal "straight outta compton".

In this example, we're interested in actor entities which have the relation "act" with **the** entity representing the
"straight outta compton" movie.

It is possible to describe broader relationships by composing a multi-hop query such as:

```sh
(me {name:'swilly'})-[:friends_with]->()-[:friends_with]->(foaf)
```

Here we're interested in finding out who my friends' friends are.

Nodes can have more than one relationship coming in or out of them, for instance:

```sh
(me {name:'swilly'})-[:visited]->(c:country)<-[:visited]-(friend)<-[:friends_with]-({name:'swilly'})
```

Here we're interested in knowing which of my friends have visited at least one country I've been to.

#### Variable length relationships

Nodes that are a variable number of relationship→node hops away can be found using the following syntax:

```sh
-[:type*minHops..maxHops]->
```

`type`, `minHops` and `maxHops` are all optional and default to type agnostic, 1 and infinity, respectively.

When no bounds are given the dots may be omitted. The dots may also be omitted when setting only one bound and this implies a fixed length pattern.

Example:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (martin:actor { name: 'Charlie Sheen' })-[:PLAYED_WITH*1..3]->(colleague:actor)
RETURN colleague"
```

Returns all actors related to 'Charlie Sheen' by 1 to 3 hops.

#### WHERE

This clause is not mandatory, but if you want to filter results, you can specify your predicates here.

Supported operations:

- `=`
- `<>`
- `<`
- `<=`
- `>`
- `>=`
- `CONTAINS`
- `ENDS WITH`
- `IN`
- `STARTS WITH`

Predicates can be combined using AND / OR / NOT.

Be sure to wrap predicates within parentheses to control precedence.

Examples:

```sh
WHERE (actor.name = "john doe" OR movie.rating > 8.8) AND movie.votes <= 250)
```

```sh
WHERE actor.age >= director.age AND actor.age > 32
```

It is also possible to specify equality predicates within nodes using the curly braces as such:

```sh
(:president {name:"Jed Bartlett"})-[:won]->(:state)
```

Here we've required that the president node's name will have the value "Jed Bartlett".

There's no difference between inline predicates and predicates specified within the WHERE clause.

#### RETURN

In its simple form, Return defines which properties the returned result-set will contain.

Its structure is a list of `alias.property` separated by commas.

For convenience, it's possible to specify the alias only when you're interested in every attribute an entity possesses,
and don't want to specify each attribute individually. For example:

```sh
RETURN movie.title, actor
```

Use the DISTINCT keyword to remove duplications within the result-set:

```sh
RETURN DISTINCT friend_of_friend.name
```

In the above example, suppose we have two friends, Joe and Miesha,
and both know Dominick.

DISTINCT will make sure Dominick will only appear once
in the final result set.


Return can also be used to aggregate data, similar to group by in SQL.

Once an aggregation function is added to the return
list, all other "none" aggregated values are considered as group keys, for example:

```sh
RETURN movie.title, MAX(actor.age), MIN(actor.age)
```

Here we group data by movie title and for each movie, and we find its youngest and oldest actor age.

#### Aggregations

Supported aggregation functions include:

- `avg`
- `collect`
- `count`
- `max`
- `min`
- `percentileCont`
- `percentileDisc`
- `stDev`
- `sum`

#### ORDER BY

Order by specifies that the output be sorted and how.

You can order by multiple properties by stating each variable in the ORDER BY clause.

The result will be sorted by the first variable listed.

For equal values, it will go to the next property in the ORDER BY
clause, and so on.

```sh
ORDER BY <alias.property list> [ASC/DESC]
```

Below we sort our friends by height. For equal heights, weight is used to break ties.

```sh
ORDER BY friend.height, friend.weight DESC
```

#### SKIP

The optional skip clause allows a specified number of records to be omitted from the result set.

```sh
SKIP <number of records to skip>
```

This can be useful when processing results in batches. A query that would examine the second 100-element batch of nodes with the label `person`, for example, would be:

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (p:person) RETURN p ORDER BY p.name SKIP 100 LIMIT 100"
```

#### LIMIT

Although not mandatory, you can use the limit clause
to limit the number of records returned by a query:

```sh
LIMIT <max records to return>
```

If not specified, there's no limit to the number of records returned by a query.

#### CREATE

CREATE is used to introduce new nodes and relationships.

The simplest example of CREATE would be a single node creation:

```sh
CREATE (n)
```

It's possible to create multiple entities by separating them with a comma.

```sh
CREATE (n),(m)
```

```sh
CREATE (:person {name: 'Kurt', age:27})
```

To add relations between nodes, in the following example we first find an existing source node. After it's found, we create a new relationship and destination node.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH(a:person)
WHERE a.name = 'Kurt'
CREATE (a)-[:member]->(:band {name:'Nirvana'})"
```

Here the source node is a bounded node, while the destination node is unbounded.

As a result, a new node is created representing the band Nirvana and a new relation connects Kurt to the band.

Lastly we create a complete pattern.

All entities within the pattern which are not bounded will be created.

```sh
GRAPH.QUERY DEMO_GRAPH
"CREATE (jim:person{name:'Jim', age:29})-[:friends]->(pam:person {name:'Pam', age:27})-[:works]->(:employer {name:'Dunder Mifflin'})"
```

This query will create three nodes and two relationships.

#### DELETE

DELETE is used to remove both nodes and relationships.

Note that deleting a node also deletes all of its incoming and outgoing relationships.

To delete a node and all of its relationships:

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (p:person {name:'Jim'}) DELETE p"
```

To delete relationship:

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (:person {name:'Jim'})-[r:friends]->() DELETE r"
```

This query will delete all `friend` outgoing relationships from the node with the name 'Jim'.

#### SET

SET is used to create or update properties on nodes and relationships.

To set a property on a node, use `SET`.

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (n { name: 'Jim' }) SET n.name = 'Bob'"
```

If you want to set multiple properties in one go, simply separate them with a comma to set multiple properties using a single SET clause.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (n { name: 'Jim', age:32 })
SET n.age = 33, n.name = 'Bob'"
```

To remove a node's property, simply set property value to NULL.

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (n { name: 'Jim' }) SET n.name = NULL"
```

#### MERGE

The MERGE clause ensures that a pattern exists in the graph (either the pattern already exists, or it needs to be created).

Currently, MERGE only functions as a standalone clause so it cannot be combined with other directives such as MATCH or RETURN.

MERGE either matches existing nodes and binds them, or it creates new data and binds that.

It’s like a combination of MATCH and CREATE that also allows you to specify what happens if the data was matched or created.

For example, you can specify that the graph must contain a node for a user with a certain name.

If there isn’t a node with the correct name, a new node will be created and its name property set.

When using MERGE on full patterns, either the whole pattern matches or the whole pattern is created.

MERGE will not partially use existing patterns — it’s all or nothing.

To merge a single node with a label:

```sh
GRAPH.QUERY DEMO_GRAPH "MERGE (robert:Critic)"
```

To merge a single node with properties:

```sh
GRAPH.QUERY DEMO_GRAPH "MERGE (charlie { name: 'Charlie Sheen', age: 10 })"
```

To merge a single node, specifying both label and property:

```sh
GRAPH.QUERY DEMO_GRAPH "MERGE (michael:Person { name: 'Michael Douglas' })""
```

To merge on a relation:

```sh
GRAPH.QUERY DEMO_GRAPH
"MERGE (charlie { name: 'Charlie Sheen', age: 10 })-[r:ACTED_IN]->(wallStreet:MOVIE)"
```
#### WITH
The WITH clause allows parts of queries to be independently executed and have their results handled uniquely.

This allows for more flexible query composition as well as data manipulations that would otherwise not be possible in a single query.

If, for example, we wanted to find all children in our graph who are above the average age of all people:
```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (p:Person) WITH AVG(p.age) AS average_age MATCH (:Person)-[:PARENT_OF]->(child:Person) WHERE child.age > average_age return child
```

This also allows us to use modifiers like `DISTINCT`, `SKIP`, `LIMIT`, and `ORDER` that otherwise require `RETURN` clauses.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (u:User)  WITH u AS nonrecent ORDER BY u.lastVisit LIMIT 3 SET nonrecent.should_contact = true"
```

Extended `WITH` functionality is currently in development, see [known limitations](known_limitations.md).

#### UNWIND
The UNWIND clause breaks down a given list into a sequence of records; each contains a single element in the list.

The order of the records preserves the original list order.

```sh
GRAPH.QUERY DEMO_GRAPH
"CREATE (p {array:[1,2,3]})"
```

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (p) UNWIND p.array AS y RETURN y"
```

### Functions

This section contains information on all supported functions from the Cypher query language.

* [Predicate functions](#predicate-functions)
* [Scalar functions](#scalar-functions)
* [Aggregating functions](#aggregating-functions)
* [List functions](#list-functions)
* [Mathematical functions](#mathematical-functions)
* [String functions](#string-functions)
* [Node functions](#node-functions)

## Predicate functions

|Function | Description|
| ------- |:-----------|
|exists() | Returns true if the specified property exists in the node or relationship. |

## Scalar functions

|Function | Description|
| ------- |:-----------|
|id() | Returns the internal ID of a relationship or node (which is not immutable.) |
|labels() | Returns a string representation of the label of a node. |
|timestamp() | Returns the the amount of milliseconds since epoch. |
|type() | Returns a string representation of the type of a relation. |

## Aggregating functions

|Function | Description|
| ------- |:-----------|
|avg() | Returns the average of a set of numeric values|
|collect() | Returns a list containing all elements which evaluated from a given expression|
|count() | Returns the number of values or rows|
|max() | Returns the maximum value in a set of values|
|min() | Returns the minimum value in a set of values|
|sum() | Returns the sum of a set of numeric values|
|percentileDisc() | Returns the percentile of the given value over a group, with a percentile from 0.0 to 1.0|
|percentileCont() | Returns the percentile of the given value over a group, with a percentile from 0.0 to 1.0|
|stDev() | Returns the standard deviation for the given value over a group|

## List functions
|Function| Description|
| ------- |:-----------|
| head()  | return the first member of a list |
| range() | create a new list of integers in the range of [start, end]. If an interval was given, the interval between two consecutive list members will be this interval.|
| size()  | return a list size |
| tail()  | return a sublist of a list, which contains all the values withiout the first value |

## Mathematical functions

|Function | Description|
| ------- |:-----------|
|abs() | Returns the absolute value of a number|
|ceil() | Returns the smallest floating point number that is greater than or equal to a number and equal to a mathematical integer |
|floor() | Returns the largest floating point number that is less than or equal to a number and equal to a mathematical integer |
|rand() | Returns a random floating point number in the range from 0 to 1; i.e. [0,1] |
|round() | Returns the value of a number rounded to the nearest integer |
|sign() | Returns the signum of a number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number |

## String functions

|Function | Description|
| ------- |:-----------|
|left() | Returns a string containing the specified number of leftmost characters of the original string |
|lTrim() | Returns the original string with leading whitespace removed |
|reverse() | Returns a string in which the order of all characters in the original string are reversed |
|right() | Returns a string containing the specified number of rightmost characters of the original string |
|rTrim() | Returns the original string with trailing whitespace removed |
|substring() | Returns a substring of the original string, beginning with a 0-based index start and length |
|toLower() | Returns the original string in lowercase |
|toString() | Converts an integer, float or boolean value to a string |
|toUpper() | Returns the original string in uppercase |
|trim() | Returns the original string with leading and trailing whitespace removed |

## Node functions
|Function | Description|
| ------- |:-----------|
|indegree() | Returns the number of node's incoming edges. |
|outdegree() | Returns the number of node's outgoing edges. |

## Procedures
Procedures are invoked using the syntax:
```sh
GRAPH.QUERY social "CALL db.labels()"
```

Or the variant:
```sh
GRAPH.QUERY social "CALL db.labels() YIELD label"
```

YIELD modifiers are only required if explicitly specified; by default the value in the 'Yields' column will be emitted automatically.

|Procedure | Arguments | Yields | Description|
| -------  |:-------|:-------|:-----------|
|db.labels() | none | `label` | Yields all node labels in the graph. |
|db.relationshipTypes() | none | `relationshipType` | Yields all relationship types in the graph. |
|db.propertyKeys() | none | `propertyKey` | Yields all property keys in the graph. |
|db.idx.fulltext.createNodeIndex | `label`, `property` [, `property` ...] | none | Builds a full-text searchable index on a label and the 1 or more specified properties. |
|db.idx.fulltext.drop | `label` | none | Deletes the full-text index associated with the given label. |
|db.idx.fulltext.queryNodes | `label`, `string` | `node` | Retrieve all nodes that contain the specified string in the full-text indexes on the given label. |

## Indexing
RedisGraph supports single-property indexes for node labels.
The creation syntax is:

```sh
GRAPH.QUERY DEMO_GRAPH "CREATE INDEX ON :person(age)"
```

After an index is explicitly created, it will automatically be used by queries that reference that label and any indexed property in a filter.

```sh
GRAPH.EXPLAIN G "MATCH (p:person) WHERE p.age > 80 RETURN p"
1) "Results"
2) "    Project"
3) "        Index Scan | (p:person)"
```

This can significantly improve the runtime of queries with very specific filters. An index on `:employer(name)`, for example, will dramatically benefit the query:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (:employer {name: 'Dunder Mifflin'})-[:employs]->(p:person) RETURN p"
```

RedisGraph can use multiple indexes as ad-hoc composite indexes at query time. For example, if `age` and `years_employed` are both indexed, then both indexes will be utilized in the query:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (p:person) WHERE p.age < 30 OR p.years_employed < 3 RETURN p"
```

Individual indexes can be deleted using the matching syntax:

```sh
GRAPH.QUERY DEMO_GRAPH "DROP INDEX ON :person(age)"
```

## Full-text indexes

RedisGraph leverages the indexing capabilities of RediSearch to provide full-text indexes through procedure calls. To construct a full-text index on the `title` property of all nodes with label `movie`, use the syntax:

```sh
GRAPH.QUERY DEMO_GRAPH "CALL db.idx.fulltext.createNodeIndex('movie', 'title')"
```

(More properties can be added to this index by adding their names to the above set of arguments, or using this syntax again with the additional names.)

Now this index can be invoked to match any whole words contained within:

```sh
GRAPH.QUERY DEMO_GRAPH
"CALL db.idx.fulltext.queryNodes('movie', 'Book') YIELD node RETURN node.title"
1) 1) "node.title"
2) 1) 1) "The Jungle Book"
   2) 1) "The Book of Life"
3) 1) "Query internal execution time: 0.927409 milliseconds"
```

This CALL clause can be interleaved with other Cypher clauses to perform more elaborate manipulations:
```sh
GRAPH.QUERY DEMO_GRAPH
"CALL db.idx.fulltext.queryNodes('movie', 'Book') YIELD node AS m
WHERE m.genre = 'Adventure'
RETURN m ORDER BY m.rating"
1) 1) "m"
2) 1) 1) 1) 1) "id"
            2) (integer) 1168
         2) 1) "labels"
            2) 1) "movie"
         3) 1) "properties"
            2) 1) 1) "genre"
                  2) "Adventure"
               2) 1) "rating"
                  2) "7.6"
               3) 1) "votes"
                  2) (integer) 151342
               4) 1) "year"
                  2) (integer) 2016
               5) 1) "title"
                  2) "The Jungle Book"
3) 1) "Query internal execution time: 0.226914 milliseconds"
```

## GRAPH.DELETE

Completely removes the graph and all of its entities.

Arguments: `Graph name`

Returns: `String indicating if operation succeeded or failed.`

```sh
GRAPH.DELETE us_government
```

Note: To delete a node from the graph (not the entire graph), execute a `MATCH` query and pass the alias to the `DELETE` clause:

```
GRAPH.QUERY DEMO_GRAPH "MATCH (x:y {propname: propvalue}) DELETE x"
```

WARNING: When you delete a node, all of the node's incoming/outgoing relationships are also removed.

## GRAPH.EXPLAIN

Constructs a query execution plan but does not run it. Inspect this execution plan to better
understand how your query will get executed.

Arguments: `Graph name, Query`

Returns: `String representation of a query execution plan`

```sh
GRAPH.EXPLAIN us_government "MATCH (p:president)-[:born]->(h:state {name:'Hawaii'}) RETURN p"
```

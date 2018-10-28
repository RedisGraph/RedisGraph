# Redis Graph Commands

## GRAPH.QUERY

Executes the given query against a specified graph.

Arguments: `Graph name, Query`

Returns: `Result set`

```sh
GRAPH.QUERY us_government "MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
```

### Query language

The syntax is based on [openCypher](http://www.opencypher.org/), and only a subset of the language currently
supported.

1. [Clauses](#query-structure)
2. [Functions](#functions)

### Query structure

- MATCH
- WHERE
- RETURN
- ORDER BY
- LIMIT
- CREATE
- MERGE
- DELETE
- SET

#### MATCH

Match describes the relationship between queried entities, using ascii art to represent pattern(s) to match against.

Nodes are represented by parenthesis `()`,
while edges are represented by brackets `[]`.

Each graph entity node/edge can contain an alias and a label, but both can be left empty if needed.

Entity structure: `alias:label {filters}`.

Alias, label and filters are all optional.

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

Nodes can have more than one edge coming in or out of them, for instance:

```sh
(me {name:'swilly'})-[:visited]->(c:country)<-[:visited]-(friend)<-[:friends_with]-({name:'swilly'})
```

Here we're interested in knowing which of my friends have visited at least one country I've been to.

#### Variable length relationships

Nodes that are a variable number of relationship→node hops away can be found using the following syntax:

```sh
-[:type*minHops..maxHops]→
```

`type`, `minHops` and `maxHops` are all optional and default to: type agnostic, 1 and infinity respectively.

When no bounds are given the dots may be omitted. The dots may also be omitted when setting only one bound and this implies a fixed length pattern.

Example:

```sh
MATCH (martin:actor { name: 'Charlie Sheen' })-[:PLAYED_WITH*1..3]-(colleague:actor)
RETURN colleague
```

Returns all actors related to 'Charlie Sheen' by 1 to 3 hops.

#### WHERE

This clause is not mandatory, but if you want to filter results, you can specify your predicates here.

Supported operations:

- `=`
- `!=`
- `<`
- `<=`
- `>`
- `>=`

Predicates can be combined using AND / OR.

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
(:president {name:"Barack Obama"})-[:won]->(:state)
```

Here we've required that the president node's name will have the value "Barack Obama".

There's no difference between inlined predicates and predicates specified within the WHERE clause.

#### RETURN

In its simple form, Return defines which properties the returned result-set will contain.

Its structure is a list of `alias.property` separated by commas.

For convenience, it's possible to specify the alias only when you're interested in every attribute an entity possesses,
and don't want to specify each attribute individually. e.g.

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


Return can also be used to aggregate data similar to SQL group by.

Once an aggregation function is added to the return
list, all other "none" aggregated values are considered as group keys, for example:

```sh
RETURN movie.title, MAX(actor.age), MIN(actor.age)
```

Here we group data by movie title and for each movie, we find its youngest and oldest actor age.

#### Aggregations

Supported aggregation functions include:

- `sum`
- `avg`
- `min`
- `max`
- `count`
- `percentileCont`
- `percentileDisc`
- `stDev`

#### ORDER BY

Order by specifies that the output should be sorted and how.

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

#### LIMIT

Although not mandatory, in order to limit the number of records returned by a query, you can
use the limit clause:

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

To add relations between nodes, in the following example we first locate an existing source node, and once found we create a new relationship and destination node.

```sh
MATCH(a:person)
WHERE a.name = 'Kurt'
CREATE (a)-[:member]->(:band {name:"Nirvana"})
```

Here the source node is a bounded node, while the destination node is unbounded.

As a result, a new node is created representing the band Nirvana and a new relation connects Kurt to the band.

Lastly we'll create a complete pattern.

All entities within the pattern which are not bounded will be created.

```sh
CREATE (jim:person{name:'Jim', age:29})-[friends]->(pam:person {name:'Pam', age:27})-[works]->(:employer {name:'Dunder Mifflin'})
```

This query will create three nodes and two relationships.

#### DELETE

DELETE is used to remove both nodes and relationships.

Please remember that deleting a node will also delete all of its incoming and outgoing relationships.

To delete a node and all of its relationships:

```sh
MATCH (p:person {name:'Jim'})
DELETE p
```

To delete relationship:

```sh
MATCH (p:person {name:'Jim'})-[r:friends]->()
DELETE r
```

This query will delete all `friend` outgoing relationships from the node with the name 'Jim'.

#### SET

SET is used to create or update properties on nodes and edges.

To set a property on a node, use `SET`.

```sh
MATCH (n { name: 'Jim' }) SET n.name = 'Bob'
```

If you want to set multiple properties in one go, simply separate them with a comma to set multiple properties using a single SET clause.

```sh
MATCH (n { name: 'Jim', age:32 })
SET n.age = 33, n.name = 'Bob'
```

To remove a node's property, simply set property value to NULL.

```sh
MATCH (n { name: 'Jim' }) SET n.name = NULL
```

#### MERGE

The MERGE clause ensures that a pattern exists in the graph (either the pattern already exists, or it needs to be created).

MERGE either matches existing nodes and binds them, or it creates new data and binds that.

It’s like a combination of MATCH and CREATE that additionally allows you to specify what happens if the data was matched or created.

For example, you can specify that the graph must contain a node for a user with a certain name.

If there isn’t a node with the correct name, a new node will be created and its name property set.

When using MERGE on full patterns, either the whole pattern matches, or the whole pattern is created.

MERGE will not partially use existing patterns — it’s all or nothing.

To merge a single node with a label:

```sh
MERGE (robert:Critic)
```

To merge a single node with properties:

```sh
MERGE (charlie { name: 'Charlie Sheen', age: 10 })
```

To merge a single node, specifying both label and property:

```sh
MERGE (michael:Person { name: 'Michael Douglas' })
```

To merge on a relation:

```sh
MERGE (charlie { name: 'Charlie Sheen', age: 10 })-[r:ACTED_IN]->(wallStreet:MOVIE)
```

### Functions

This section contains information on all supported functions from the openCypher query language.

* [Aggregating functions](#aggregating-functions)
* [Mathematical functions](#mathematical-functions)
* [String functions](#string-functions)
* [Scalar functions](#scalar-functions)

## Aggregating functions

|Function | Description|
| ------- |:-----------|
|avg() | Returns the average of a set of numeric values.|
|count() | Returns the number of values or rows.|
|max() | Returns the maximum value in a set of values.|
|min() | Returns the minimum value in a set of values.|
|sum() | Returns the sum of a set of numeric values.|
|percentileDisc() | Returns the percentile of the given value over a group, with a percentile from 0.0 to 1.0.|
|percentileCont() | Returns the percentile of the given value over a group, with a percentile from 0.0 to 1.0.|
|stDev() | Returns the standard deviation for the given value over a group.|

## Mathematical functions

|Function | Description|
| ------- |:-----------|
|abs() | Returns the absolute value of a number.|
|ceil() | Returns the smallest floating point number that is greater than or equal to a number and equal to |a mathematical integer.
|floor() | Returns the largest floating point number that is less than or equal to a number and equal to a |mathematical integer.
|rand() | Returns a random floating point number in the range from 0 to 1; i.e. [0,1].
|round() | Returns the value of a number rounded to the nearest integer.|
|sign() | Returns the signum of a number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number.|

## String functions

|Function | Description|
| ------- |:-----------|
|left() | Returns a string containing the specified number of leftmost characters of the original string.|
|lTrim() | Returns the original string with leading whitespace removed.|
|reverse() | Returns a string in which the order of all |characters in the original string have been reversed.|
|right() | Returns a string containing the specified number of rightmost characters of the original string.|
|rTrim() | Returns the original string with trailing whitespace removed.|
|substring() | Returns a substring of the original string, beginning with a 0-based index start and length.|
|toLower() | Returns the original string in lowercase.|
|toString() | Converts an integer, float or boolean value to a string.|
|toUpper() | Returns the original string in uppercase.|
|trim() | Returns the original string with leading and trailing whitespace removed.|

## Scalar functions

|Function | Description|
| ------- |:-----------|
|id() | Returns the ID of a relationship or node.|

## GRAPH.DELETE

Completely removes graph and all of its entities.

Arguments: `Graph name`

Returns: `String indicating if operation succeeded or failed.`

```sh
GRAPH.DELETE us_government
```

*Note*: if you'd like to delete a node from the graph (not the entire graph), you simply execute a `MATCH` query and pass the alias to the `DELETE` clause:

```
MATCH (x:y {propname: propvalue}) DELETE x
```

Beware that when you delete a node, all of the node's incoming/outgoing edges will also be removed.

## GRAPH.EXPLAIN

Constructs a query execution plan but does not run it. Inspect this execution plan to better
understand how your query will get executed.

Arguments: `Graph name, Query`

Returns: `String representation of a query execution plan`

```sh
GRAPH.EXPLAIN us_government "MATCH (p:president)-[:born]->(h:state {name:'Hawaii'}) RETURN p"
```

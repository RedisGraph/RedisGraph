# Redis Graph Commands

## GRAPH.EXPLAIN

Constructs a query execution plan but does not run it. Inspect this execution plan to better
understand how your query will get executed.

Arguments: `Graph name, Query`

Returns: `String representation of a query execution plan`

```sh
GRAPH.EXPLAIN us_government "MATCH (p:president)-[:born]->(h:state {name:Hawaii}) RETURN p"
```

## GRAPH.QUERY

Executes the given query against a specified graph.

Arguments: `Graph name, Query`

Returns: `Result set`

```sh
GRAPH.QUERY us_government "MATCH (p:president)-[:born]->(:state {name:Hawaii}) RETURN p"
```

### Query language

The syntax is based on [openCypher](http://www.opencypher.org/) and currently only a subset of the language is
supported.

1. [Clauses](#query-structure)
2. [Functions](#functions)

A query is composed of five parts:

### Query structure

- MATCH
- WHERE
- RETURN
- ORDER BY
- LIMIT
- CREATE
- DELETE

#### MATCH

Describes the relationship between queried entities, it is composed of three parts:

- Source node (S)
- Relationship [R]
- Destination node (D)

Combining the three together
`(S)-[R]->(D)`

Each graph entity node/edge can contain an alias and a label, but both can be left empty if needed.

Entity structure: `alias:label {filters}` alias, label and filters are all optional.

Example:

```sh
(a:actor)-[:act]->(m:movie {title:"straight outta compton"})
```

`a` is an alias for the source node, which we'll be able to refer to at different places within our query.

`actor` is the label under which this node is marked.

`act` is the relationship type.

`m` an alias for the destination node.

`movie` destination node is of "type" movie.

`{title:"straight outta compton"}` requires the node's title attribute to equal "straight outta compton".

As such, we're interested in actor entities which have the relation "act" with **the** entity representing the
"straight outta compton" movie.

It is possible to describe broader relationships by composing a multi-hop query such as:

```sh
(me {name:swilly})-[:friends_with]->()-[:friends_with]->(fof)
```

Here we're interested in finding out who are my friends' friends.

Nodes can have more than one edge coming in or out of them, for instance:

```sh
(me {name:swilly})-[:visited]->(c:country)<-[:visited]-(friend)<-[:friends_with]-({name:swilly})
```

Here we're interested in knowing which of my friends have visited at least one country i've been to.

#### WHERE

This clause is not mandatory, but in order to filter results you can define predicates of two kinds:

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

Predicates can be combined using AND / OR. Be sure to wrap predicates within parentheses to control precedence.


Examples:

```sh
WHERE (actor.name = "john doe" OR movie.rating > 8.8) AND movie.votes <= 250)
```

```sh
WHERE actor.age >= director.age AND actor.age > 32
```

It is also possible to specify equality predicates within nodes and edges using the curly braces as such:

```sh
(:president {name:"Barack Obama"})-[:won {term:2}]->(:state)
```

Here we've required that the president node's name will have the value "Barack Obama"
and the won edge term property will equal 2.

There's no difference between inlined predicates and predicates specified within the WHERE clause.

#### RETURN

In its simple form, Return defines which properties the returned result-set will contain.
Its structure is a list of `alias.property` seperated by commas.
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
and both know Dominick. Then DISTINCT will make sure that Dominick will only appear once
in the final result-set.


Return can also be used to aggregate data similar to SQL group by. Once an aggregation function is added to the return
list, all other "none" aggregated values are considered as group keys, for example:

```sh
RETURN movie.title, MAX(actor.age), MIN(actor.age)
```

Here we group data by movie title and for each movie, we find its youngest and oldest actor age.

#### Aggregations

Supported aggregation functions:

- `sum`
- `avg`
- `min`
- `max`
- `count`

#### ORDER BY

Specifies that the output should be sorted and how.

You can order by multiple properties by stating each variable in the ORDER BY clause.
The result will be sorted by the first variable listed, and for equal values, go to the next property in the ORDER BY
clause, and so on.

```sh
ORDER BY <alias.property list> [ASC/DESC]
```

Below we sort our friends by height. For similar heights, weight is used to break even.

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

CREATE query is used to introduce new nodes and relationships.
The simplest example of CREATE would be a single node creation:

```sh
CREATE (n)
```

It's possible to create multiple entities by seperating them with a comma.

```sh
CREATE (n),(m)
```

Label and properties can be specified at creation time

```sh
CREATE (:person {name: Kurt, age:27})
```

Adding relations between nodes, in the following example we first locate an existing source node,
once found we create a new relationship and destination node.

```sh
MATCH(a:person)
WHEREE a.name = 'Kurt'
CREATE (a)-[member {position:"lead singer"}]->(:band {name:Nirvana})
RETURN
```

Here the source node is a bounded node while the destination node is unbounded,
as a result a new node is created representing the band Nirvana and a new relation connects Kurt as the lead singer of the band.

Lastly we'll create a complete pattern, all entities within the pattern which are not bounded will be created.
```sh
CREATE 
(jim:person{name:'Jim', age:29})-[friends]->(pam:person {name:'Pam', age:27})-[works]->(:employer {name:'Dunder Mifflin'})
RETURN
```
This query will create three nodes and two relationships.

#### DELETE
DELETE query is used to remove both nodes and relationships, please remember that deleting a node will also delete
all of its incoming and outgoing relationships.

Delete a node and all of its relationships:
```sh
MATCH (p:person {name:'Jim'})
DELETE p
```

Delete a relationship
```sh
MATCH (p:person {name:'Jim'})-[r:friends]->()
DELETE r
```
This query will delete all `friend` outgoing relationships from the node with the name 'Jim'.

### Functions
This section contains information on all supported functions from the OpenCypher query language.

* [Aggregating functions](#aggregating-functions)
* [Mathematical functions](#mathematical-functions)
* [String functions](#string-functions)

## Aggregating functions

|Function | Description|
| ------- |:-----------|
|avg() | Returns the average of a set of numeric values.|
|count() | Returns the number of values or rows.|
|max() | Returns the maximum value in a set of values.|
|min() | Returns the minimum value in a set of values.|
|sum() | Returns the sum of a set of numeric values.|


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
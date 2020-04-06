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
- OPTIONAL MATCH
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
- UNION

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

##### Variable length relationships

Nodes that are a variable number of relationship→node hops away can be found using the following syntax:

```sh
-[:type*minHops..maxHops]->
```

`type`, `minHops` and `maxHops` are all optional and default to type agnostic, 1 and infinity, respectively.

When no bounds are given the dots may be omitted. The dots may also be omitted when setting only one bound and this implies a fixed length pattern.

Example:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (charlie:actor { name: 'Charlie Sheen' })-[:PLAYED_WITH*1..3]->(colleague:actor)
RETURN colleague"
```

Returns all actors related to 'Charlie Sheen' by 1 to 3 hops.

##### Bidirectional path traversal

If a relationship pattern does not specify a direction, it will match regardless of which node is the source and which is the destination:
```sh
-[:type]-
```

Example:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (person_a:Person)-[:KNOWS]-(person_b:Person)
RETURN person_a, person_b"
```

Returns all pairs of people connected by a `KNOWS` relationship. Note that each pair will be returned twice, once with each node in the `person_a` field and once in the `person_b` field.

The syntactic sugar `(person_a)<-[:KNOWS]->(person_b)` will return the same results.

The bracketed edge description can be omitted if all relations should be considered: `(person_a)--(person_b)`.

##### Named paths

Named path variables are created by assigning a path in a MATCH clause to a single alias with the syntax:
`MATCH named_path = (path)-[to]->(capture)`

The named path includes all entities in the path, regardless of whether they have been explicitly aliased. Named paths can be accessed using [designated built-in functions](#path-functions) or returned directly if using a language-specific client.

Example:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH p=(charlie:actor { name: 'Charlie Sheen' })-[:PLAYED_WITH*1..3]->(:actor)
RETURN nodes(p) as actors"
```

This query will produce all the paths matching the pattern contained in the named path `p`. All of these paths will share the same starting point, the actor node representing Charlie Sheen, but will otherwise vary in length and contents. Though the variable-length traversal and `(:actor)` endpoint are not explicitly aliased, all nodes and edges traversed along the path will be included in `p`. In this case, we are only interested in the nodes of each path, which we'll collect using the built-in function `nodes()`. The returned value will contain, in order, Charlie Sheen, between 0 and 2 intermediate nodes, and the unaliased endpoint.

#### OPTIONAL MATCH

The OPTIONAL MATCH clause is a MATCH variant that produces null values for elements that do not match successfully, rather than the all-or-nothing logic for patterns in MATCH clauses.

It can be considered to fill the same role as LEFT/RIGHT JOIN does in SQL, as MATCH entities must be resolved but nodes and edges introduced in OPTIONAL MATCH will be returned as nulls if they cannot be found.

OPTIONAL MATCH clauses accept the same patterns as standard MATCH clauses, and may similarly be modified by WHERE clauses.

Multiple MATCH and OPTIONAL MATCH clauses can be chained together, though a mandatory MATCH cannot follow an optional one.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (p:Person) OPTIONAL MATCH (p)-[w:WORKS_AT]->(c:Company)
WHERE w.start_date > 2016
RETURN p, w, c"
```

All `Person` nodes are returned, as well as any `WORKS_AT` relations and `Company` nodes that can be resolved and satisfy the `start_date` constraint. For each `Person` that does not resolve the optional pattern, the person will be returned as normal and the non-matching elements will be returned as null.

Cypher is lenient in its handling of null values, so actions like property accesses and function calls on null values will return null values rather than emit errors.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (p:Person) OPTIONAL MATCH (p)-[w:WORKS_AT]->(c:Company)
RETURN p, w.department, ID(c) as ID"
```

In this case, `w.department` and `ID` will be returned if the OPTIONAL MATCH was successful, and will be null otherwise.

Clauses like SET, CREATE, MERGE, and DELETE will ignore null inputs and perform the expected updates on real inputs. One exception to this is that attempting to create a relation with a null endpoint will cause an error:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (p:Person) OPTIONAL MATCH (p)-[w:WORKS_AT]->(c:Company)
CREATE (c)-[:NEW_RELATION]->(:NEW_NODE)"
```

If `c` is null for any record, this query will emit an error. In this case, no changes to the graph are committed, even if some values for `c` were resolved.

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

```
WHERE (actor.name = "john doe" OR movie.rating > 8.8) AND movie.votes <= 250)
```

```
WHERE actor.age >= director.age AND actor.age > 32
```

It is also possible to specify equality predicates within nodes using the curly braces as such:

```
(:president {name:"Jed Bartlett"})-[:won]->(:state)
```

Here we've required that the president node's name will have the value "Jed Bartlett".

There's no difference between inline predicates and predicates specified within the WHERE clause.

It is also possible to filter on graph patterns. The following queries, which return all presidents and the states they won in, produce the same results:

```sh
MATCH (p:president), (s:state) WHERE (p)-[:won]->(s) RETURN p, s
```

and

```sh
MATCH (p:president)-[:won]->(s:state) RETURN p, s
```

Pattern predicates can be also negated and combined with the logical operators AND, OR, and NOT. The following query returns all the presidents that did not win in the states where they were governors:

```sh
MATCH (p:president), (s:state) WHERE NOT (p)-[:won]->(s) AND (p)->[:governor]->(s) RETURN p, s
```

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

Each property may specify its sort order with `ASC`/`ASCENDING` or `DESC`/`DESCENDING`. If no order is specified, it defaults to ascending.

The result will be sorted by the first variable listed.

For equal values, it will go to the next property in the ORDER BY clause, and so on.

```sh
ORDER BY <alias.property [ASC/DESC] list>
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

```
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

The MERGE clause ensures that a path exists in the graph (either the path already exists, or it needs to be created).

MERGE either matches existing nodes and binds them, or it creates new data and binds that.

It’s like a combination of MATCH and CREATE that also allows you to specify what happens if the data was matched or created.

For example, you can specify that the graph must contain a node for a user with a certain name.

If there isn’t a node with the correct name, a new node will be created and its name property set.

Any aliases in the MERGE path that were introduced by earlier clauses can only be matched; MERGE will not create them.

When the MERGE path doesn't rely on earlier clauses, the whole path will always either be matched or created.

If all path elements are introduced by MERGE, a match failure will cause all elements to be created, even if part of the match succeeded.

The MERGE path can be followed by ON MATCH SET and ON CREATE SET directives to conditionally set properties depending on whether or not the match succeeded.

**Merging nodes**

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

**Merging paths**

Because MERGE either matches or creates a full path, it is easy to accidentally create duplicate nodes.

For example, if we run the following query on our sample graph:

```sh
GRAPH.QUERY DEMO_GRAPH
"MERGE (charlie { name: 'Charlie Sheen '})-[r:ACTED_IN]->(wallStreet:Movie { name: 'Wall Street' })"
```

Even though a node with the name 'Charlie Sheen' already exists, the full pattern does not match, so 1 relation and 2 nodes - including a duplicate 'Charlie Sheen' node - will be created.

We should use multiple MERGE clauses to merge a relation and only create non-existent endpoints:

```sh
GRAPH.QUERY DEMO_GRAPH
"MERGE (charlie { name: 'Charlie Sheen' })
 MERGE (wallStreet:Movie { name: 'Wall Street' })
 MERGE (charlie)-[r:ACTED_IN]->(wallStreet)"
```

If we don't want to create anything if pattern elements don't exist, we can combine MATCH and MERGE clauses. The following query merges a relation only if both of its endpoints already exist:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (charlie { name: 'Charlie Sheen' })
 MATCH (wallStreet:Movie { name: 'Wall Street' })
 MERGE (charlie)-[r:ACTED_IN]->(wallStreet)"
```

**On Match and On Create directives**

Using ON MATCH and ON CREATE, MERGE can set properties differently depending on whether a pattern is matched or created.

In this query, we'll merge paths based on a list of properties and conditionally set a property when creating new entities:

```sh
GRAPH.QUERY DEMO_GRAPH
"UNWIND ['Charlie Sheen', 'Michael Douglas', 'Tamara Tunie'] AS actor_name
 MATCH (movie:Movie { name: 'Wall Street' })
 MERGE (person {name: actor_name})-[:ACTED_IN]->(movie)
 ON CREATE SET person.first_role = movie.name"
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

#### UNION
The UNION clause is used to combine the result of multiple queries.

UNION combines the results of two or more queries into a single result set that includes all the rows that belong to all queries in the union.

The number and the names of the columns must be identical in all queries combined by using UNION.

To keep all the result rows, use UNION ALL.

Using just UNION will combine and remove duplicates from the result set.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (n:Actor) RETURN n.name AS name
UNION ALL
MATCH (n:Movie) RETURN n.title AS name"
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
| head()  | Return the first member of a list |
| range() | Create a new list of integers in the range of [start, end]. If an interval was given, the interval between two consecutive list members will be this interval.|
| size()  | Return a list size |
| tail()  | Return a sublist of a list, which contains all the values withiout the first value |

## Mathematical functions

|Function | Description|
| ------- |:-----------|
|abs() | Returns the absolute value of a number|
|ceil() | Returns the smallest floating point number that is greater than or equal to a number and equal to a mathematical integer |
|floor() | Returns the largest floating point number that is less than or equal to a number and equal to a mathematical integer |
|rand() | Returns a random floating point number in the range from 0 to 1; i.e. [0,1] |
|round() | Returns the value of a number rounded to the nearest integer |
|sign() | Returns the signum of a number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number |
|toInteger() | Converts a floating point or string value to an integer value. |

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

## Path functions
|Function | Description|
| ------- |:-----------|
| nodes() | Return a new list of nodes, of a given path. |
| relationships() | Return a new list of edges, of a given path. |
| length() | Return the length (number of edges) of the path|

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
|db.labels | none | `label` | Yields all node labels in the graph. |
|db.relationshipTypes | none | `relationshipType` | Yields all relationship types in the graph. |
|db.propertyKeys | none | `propertyKey` | Yields all property keys in the graph. |
|db.idx.fulltext.createNodeIndex | `label`, `property` [, `property` ...] | none | Builds a full-text searchable index on a label and the 1 or more specified properties. |
|db.idx.fulltext.drop | `label` | none | Deletes the full-text index associated with the given label. |
|db.idx.fulltext.queryNodes | `label`, `string` | `node` | Retrieve all nodes that contain the specified string in the full-text indexes on the given label. |
|algo.pageRank | `label`, `relationship-type` | `node`, `score` | Runs the pagerank algorithm over nodes of given label, considering only edges of given relationship type. |

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

RedisGraph leverages the indexing capabilities of [RediSearch](https://oss.redislabs.com/redisearch/index.html) to provide full-text indices through procedure calls. To construct a full-text index on the `title` property of all nodes with label `movie`, use the syntax:

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

## GRAPH.SLOWLOG

Returns a list containing up to 10 of the slowest queries issued against given graph id.

Each item in the list has the following structure:
1. A unix timestamp at which the logged was processed.
2. The issued command.
3. The issued query.
4. The amount of time needed for its execution, in milliseconds.

```sh
GRAPH.SLOWLOG graph_id
 1) 1) "1581932396"
    2) "GRAPH.QUERY"
    3) "MATCH (a:person)-[:friend]->(e) RETURN e.name"
    4) "0.831"
 2) 1) "1581932396"
    2) "GRAPH.QUERY"
    3) "MATCH (ME:person)-[:friend]->(:person)-[:friend]->(fof:person) RETURN fof.name"
    4) "0.288"
```

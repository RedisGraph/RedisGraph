Executes the given query against a specified graph.

Arguments: `Graph name, Query, Timeout [optional]`

Returns: [Result set](/redisgraph/design/result_structure)

```sh
GRAPH.QUERY us_government "MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
```

Query-level timeouts can be set as described in [the configuration section](/redisgraph/configuration#timeout).

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

Nodes are represented by parentheses `()`,
and Relationships are represented by brackets `[]`.

Each graph entity node/relationship can contain an alias and a label/relationship type, but both can be left empty if necessary.

Entity structure: `alias:label {filters}`.

Alias, label/relationship type, and filters are all optional.

Example:

```sh
(a:Actor)-[:ACT]->(m:Movie {title:"straight outta compton"})
```

`a` is an alias for the source node, which we'll be able to refer to at different places within our query.

`Actor` is the label under which this node is marked.

`ACT` is the relationship type.

`m` is an alias for the destination node.

`Movie` destination node is of "type" movie.

`{title:"straight outta compton"}` requires the node's title attribute to equal "straight outta compton".

In this example, we're interested in actor entities which have the relation "act" with **the** entity representing the
"straight outta compton" movie.

It is possible to describe broader relationships by composing a multi-hop query such as:

```sh
(me {name:'swilly'})-[:FRIENDS_WITH]->()-[:FRIENDS_WITH]->(foaf)
```

Here we're interested in finding out who my friends' friends are.

Nodes can have more than one relationship coming in or out of them, for instance:

```sh
(me {name:'swilly'})-[:VISITED]->(c:Country)<-[:VISITED]-(friend)<-[:FRIENDS_WITH]-(me)
```

Here we're interested in knowing which of my friends have visited at least one country I've been to.

##### Variable length relationships

Nodes that are a variable number of relationship→node hops away can be found using the following syntax:

```sh
-[:TYPE*minHops..maxHops]->
```

`TYPE`, `minHops` and `maxHops` are all optional and default to type agnostic, 1 and infinity, respectively.

When no bounds are given the dots may be omitted. The dots may also be omitted when setting only one bound and this implies a fixed length pattern.

Example:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (charlie:Actor { name: 'Charlie Sheen' })-[:PLAYED_WITH*1..3]->(colleague:Actor)
RETURN colleague"
```

Returns all actors related to 'Charlie Sheen' by 1 to 3 hops.

##### Bidirectional path traversal

If a relationship pattern does not specify a direction, it will match regardless of which node is the source and which is the destination:
```sh
-[:TYPE]-
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
"MATCH p=(charlie:Actor { name: 'Charlie Sheen' })-[:PLAYED_WITH*1..3]->(:Actor)
RETURN nodes(p) as actors"
```

This query will produce all the paths matching the pattern contained in the named path `p`. All of these paths will share the same starting point, the actor node representing Charlie Sheen, but will otherwise vary in length and contents. Though the variable-length traversal and `(:Actor)` endpoint are not explicitly aliased, all nodes and edges traversed along the path will be included in `p`. In this case, we are only interested in the nodes of each path, which we'll collect using the built-in function `nodes()`. The returned value will contain, in order, Charlie Sheen, between 0 and 2 intermediate nodes, and the unaliased endpoint.

##### All shortest paths

The `allShortestPaths` function returns all the shortest paths between a pair of entities.

`allShortestPaths()` is a MATCH mode in which only the shortest paths matching all criteria are captured. Both the source and the target nodes must be bound in an earlier WITH-demarcated scope to invoke `allShortestPaths()`.

A minimal length (must be 1) and maximal length (must be at least 1) for the search may be specified. Zero or more relationship types may be specified (e.g. [:R|Q*1..3]). No property filters may be introduced in the pattern.

`allShortestPaths()` can have any number of hops for its minimum and maximum, including zero. This number represents how many edges can be traversed in fulfilling the pattern, with a value of 0 entailing that the source node will be included in the returned path.

Filters on properties are supported, and any number of labels may be specified.

Example:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (charlie:Actor {name: 'Charlie Sheen'}), (kevin:Actor {name: 'Kevin Bacon'})
WITH charlie, kevin
MATCH p=allShortestPaths((charlie)-[:PLAYED_WITH*]->(kevin))
RETURN nodes(p) as actors"
```

This query will produce all paths of the minimum length connecting the actor node representing Charlie Sheen to the one representing Kevin Bacon. There are several 2-hop paths between the two actors, and all of these will be returned. The computation of paths then terminates, as we are not interested in any paths of length greater than 2.

##### Single-Pair minimal-weight bounded-cost bounded-length paths

(Since RedisGraph v2.10)

The `algo.SPpaths` procedure returns one, _n_, or all minimal-weight, [optionally] bounded-cost, [optionally] bounded-length distinct paths between a pair of entities. Each path is a sequence of distinct nodes connected by distinct edges.

`algo.SPpaths()` is a MATCH mode in which only the paths matching all criteria are captured. Both the source and the target nodes must be bound in an earlier WITH-demarcated scope to invoke `algo.SPpaths()`.

Input arguments:

- A map containing:
  - `sourceNode`: Mandatory. Must be of type node
  - `targetNode`: Mandatory. Must be of type node
  - `relTypes`: Optional. Array of zero or more relationship types. A relationship must have one of these types to be part of the path. If not specified or empty: the path may contain any relationship.
  - `relDirection`: Optional. string. one of `'incoming'`, `'outgoing'`, `'both'`. If not specified: `'outgoing'`.
  - `pathCount`: Optional. Number of minimal-weight paths to retrieve. Non-negative integer. If not specified: 1

    - `0`: retrieve all minimal-weight paths (all reported paths have the same weight)

      Order: 1st : minimal cost, 2nd: minimal length.

    - `1`: retrieve a single minimal-weight path

      When multiple equal-weight paths exist: (preferences: 1st : minimal cost, 2nd: minimal length)

    - _n_ > 1: retrieve up to _n_ minimal-weight paths (reported paths may have different weights)

      When multiple equal-weight paths exist: (preferences: 1st : minimal cost, 2nd: minimal length)

  - `weightProp`: Optional. If not specified: use the default weight: 1 for each relationship.

    The name of the property that represents the weight of each relationship (integer / float)

    If such property doesn’t exist, of if its value is not a positive numeric - use the default weight: 1

    Note: when all weights are equal: minimal-weight ≡ shortest-path.

  - `costProp`: Optional. If not specified: use the default cost: 1 for each relationship.

    The name of the property that represents the cost of each relationship (integer / float)

    If such property doesn't exist, or if its value is not a positive numeric - use the default cost: 1

  - `maxLen`: Optional. Maximal path length (number of relationships along the path). Positive integer. 

    If not specified: no maximal length constraint.

  - `maxCost`: Optional. Positive numeric. If not specified: no maximal cost constraint.

    The maximal cumulative cost for the relationships along the path.
    
Result:

  - Paths conforming to the input arguments. For each reported path:

    - `path` - the path

    - `pathWeight` - the path’s weight

    - `pathCost` - the path’s cost

    To retrieve additional information:

    - The path’s length can be retrieved with `length(path)`

    - An array of the nodes along the path can be retrieved with `nodes(path)`

    - The path’s first node can be retrieved with `nodes(path)[0]`

    - The path’s last node can be retrieved with `nodes(path)[-1]`

    - An array of the relationship's costs along the path can be retrieved with `[r in relationships(path) | r.cost]` where cost is the name of the cost property

    - An array of the relationship's weights along the path can be retrieved with `[r in relationships(path) | r.weight]` where weight is the name of the weight property

Behavior in presence on multiple-edges:

  - multi-edges are two or more edges connecting the same pair of vertices (possibly with different weights and costs). 

  - All matching edges are considered. Paths with identical vertices and different edges are different paths. The following are 3 different paths ('n1', 'n2', and 'n3' are nodes; 'e1', 'e2', 'e3', and 'e4' are edges): (n1)-[e1]-(n2)-[e2]-(n3),  (n1)-[e1]-(n2)-[e3]-(n3),  (n1)-[e4]-(n2)-[e3]-(n3)

Example:

```sh
GRAPH.QUERY DEMO_GRAPH 
"MATCH (s:Actor {name: 'Charlie Sheen'}), (t:Actor {name: 'Kevin Bacon'}) 
CALL algo.SPpaths( {sourceNode: s, targetNode: t, relTypes: ['r1', 'r2', 'r3'], relDirection: 'outgoing', pathCount: 1, weightProp: 'weight', costProp: 'cost', maxLen: 3, maxCost: 100} ) 
YIELD path, pathCost, pathWeight
RETURN path ORDER BY pathCost"
```

##### Single-Source minimal-weight bounded-cost bounded-length paths

(Since RedisGraph v2.10)

The `algo.SSpaths` procedure returns one, _n_, or all minimal-weight, [optionally] bounded-cost, [optionally] bounded-length distinct paths from a given entity. Each path is a sequence of distinct nodes connected by distinct edges.

`algo.SSpaths()` is a MATCH mode in which only the paths matching all criteria are captured. The source node must be bound in an earlier WITH-demarcated scope to invoke `algo.SSpaths()`.

Input arguments:

- A map containing:
  - `sourceNode`: Mandatory. Must be of type node
  - `relTypes`: Optional. Array of zero or more relationship types. A relationship must have one of these types to be part of the path. If not specified or empty: the path may contain any relationship.
  - `relDirection`: Optional. string. one of `'incoming'`, `'outgoing'`, `'both'`. If not specified: `'outgoing'`.
  - `pathCount`: Optional. Number of minimal-weight paths to retrieve. Non-negative integer. If not specified: 1

    This number is global (not per source-target pair); all returned paths may be with the same target.

    - `0`: retrieve all minimal-weight paths (all reported paths have the same weight)

      Order: 1st : minimal cost, 2nd: minimal length.

    - `1`: retrieve a single minimal-weight path

      When multiple equal-weight paths exist: (preferences: 1st : minimal cost, 2nd: minimal length)

    - _n_ > 1: retrieve up to _n_ minimal-weight paths (reported paths may have different weights)

      When multiple equal-weight paths exist: (preferences: 1st : minimal cost, 2nd: minimal length)

  - `weightProp`: Optional. If not specified: use the default weight: 1 for each relationship.

    The name of the property that represents the weight of each relationship (integer / float)

    If such property doesn’t exist, of if its value is not a positive numeric - use the default weight: 1

    Note: when all weights are equal: minimal-weight ≡ shortest-path.

  - `costProp`: Optional. If not specified: use the default cost: 1 for each relationship.

    The name of the property that represents the cost of each relationship (integer / float)

    If such property doesn't exist, or if its value is not a positive numeric - use the default cost: 1

  - `maxLen`: Optional. Maximal path length (number of relationships along the path). Positive integer. 

    If not specified: no maximal length constraint.

  - `maxCost`: Optional. Positive numeric. If not specified: no maximal cost constraint.

    The maximal cumulative cost for the relationships along the path.
    
Result:

  - Paths conforming to the input arguments. For each reported path:

    - `path` - the path

    - `pathWeight` - the path’s weight

    - `pathCost` - the path’s cost

    To retrieve additional information:

    - The path’s length can be retrieved with `length(path)`

    - An array of the nodes along the path can be retrieved with `nodes(path)`

    - The path’s first node can be retrieved with `nodes(path)[0]`

    - The path’s last node can be retrieved with `nodes(path)[-1]`

    - An array of the relationship's costs along the path can be retrieved with `[r in relationships(path) | r.cost]` where cost is the name of the cost property

    - An array of the relationship's weights along the path can be retrieved with `[r in relationships(path) | r.weight]` where weight is the name of the weight property

Behavior in presence on multiple-edges:

  - multi-edges are two or more edges connecting the same pair of vertices (possibly with different weights and costs). 

  - All matching edges are considered. Paths with identical vertices and different edges are different paths. The following are 3 different paths ('n1', 'n2', and 'n3' are nodes; 'e1', 'e2', 'e3', and 'e4' are edges): (n1)-[e1]-(n2)-[e2]-(n3), (n1)-[e1]-(n2)-[e3]-(n3), (n1)-[e4]-(n2)-[e3]-(n3)

Example:

```sh
GRAPH.QUERY DEMO_GRAPH 
"MATCH (s:Actor {name: 'Charlie Sheen'})
CALL algo.SSpaths( {sourceNode: s, relTypes: ['r1', 'r2', 'r3'], relDirection: 'outgoing', pathCount: 1, weightProp: 'weight', costProp: 'cost', maxLen: 3, maxCost: 100} ) 
YIELD path, pathCost, pathWeight
RETURN path ORDER BY pathCost"
```


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
(:President {name:"Jed Bartlett"})-[:WON]->(:State)
```

Here we've required that the president node's name will have the value "Jed Bartlett".

There's no difference between inline predicates and predicates specified within the WHERE clause.

It is also possible to filter on graph patterns. The following queries, which return all presidents and the states they won in, produce the same results:

```sh
MATCH (p:President), (s:State) WHERE (p)-[:WON]->(s) RETURN p, s
```

and

```sh
MATCH (p:President)-[:WON]->(s:State) RETURN p, s
```

Pattern predicates can be also negated and combined with the logical operators AND, OR, and NOT. The following query returns all the presidents that did not win in the states where they were governors:

```sh
MATCH (p:President), (s:State) WHERE NOT (p)-[:WON]->(s) AND (p)->[:governor]->(s) RETURN p, s
```

Nodes can also be filtered by label:

```sh
MATCH (n)-[:R]->() WHERE n:L1 OR n:L2 RETURN n 
```

When possible, it is preferable to specify the label in the node pattern of the MATCH clause.

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

This can be useful when processing results in batches. A query that would examine the second 100-element batch of nodes with the label `Person`, for example, would be:

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (p:Person) RETURN p ORDER BY p.name SKIP 100 LIMIT 100"
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
CREATE (:Person {name: 'Kurt', age: 27})
```

To add relations between nodes, in the following example we first find an existing source node. After it's found, we create a new relationship and destination node.

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (a:Person)
WHERE a.name = 'Kurt'
CREATE (a)-[:MEMBER]->(:Band {name:'Nirvana'})"
```

Here the source node is a bounded node, while the destination node is unbounded.

As a result, a new node is created representing the band Nirvana and a new relation connects Kurt to the band.

Lastly we create a complete pattern.

All entities within the pattern which are not bounded will be created.

```sh
GRAPH.QUERY DEMO_GRAPH
"CREATE (jim:Person{name:'Jim', age:29})-[:FRIENDS]->(pam:Person {name:'Pam', age:27})-[:WORKS]->(:Employer {name:'Dunder Mifflin'})"
```

This query will create three nodes and two relationships.

#### DELETE

DELETE is used to remove both nodes and relationships.

Note that deleting a node also deletes all of its incoming and outgoing relationships.

To delete a node and all of its relationships:

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (p:Person {name:'Jim'}) DELETE p"
```

To delete relationship:

```sh
GRAPH.QUERY DEMO_GRAPH "MATCH (:Person {name:'Jim'})-[r:FRIENDS]->() DELETE r"
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

The same can be accomplished by setting the graph entity variable to a map:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (n { name: 'Jim', age:32 })
SET n = {age: 33, name: 'Bob'}"
```

Using `=` in this way replaces all of the entity's previous properties, while `+=` will only set the properties it explicitly mentions.

In the same way, the full property set of a graph entity can be assigned or merged:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (jim {name: 'Jim'}), (pam {name: 'Pam'})
SET jim = pam"
```

After executing this query, the `jim` node will have the same property set as the `pam` node.

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
GRAPH.QUERY DEMO_GRAPH "MERGE (michael:Person { name: 'Michael Douglas' })"
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
* [Mathematical operators](#mathematical-operators)
* [Mathematical functions](#mathematical-functions)
* [Trigonometric functions](#trigonometric-functions)
* [String functions](#string-functions)
* [Point functions](#point-functions)
* [Type conversion functions](#type-conversion-functions)
* [Node functions](#node-functions)
* [Path functions](#path-functions)

## Predicate functions

| Function                                         | Description                                                                                              |
| -------                                          | :-----------                                                                                             |
| exists()                                         | Returns true if the specified property exists in the node or relationship                                |
| isEmpty()                                        | Returns true if the input list or map contains no elements or if the input string contains no characters |
| [any()](#existential-comprehension-functions)    | Returns true if the inner WHERE predicate holds true for at least one element in the input array         |
| [all()](#existential-comprehension-functions)    | Returns true if the inner WHERE predicate holds true for all elements in the input array                 |
| [none()](#existential-comprehension-functions)   | Returns true if the inner WHERE predicate holds false for all elements in the input array                |
| [single()](#existential-comprehension-functions) | Returns true if the inner WHERE predicate holds true for exactly one element only in the input array     |
| [CASE...WHEN](#case-when)                        | Evaluates the CASE expression and returns the value indicated by the matching WHEN statement             |

## Scalar functions

| Function               | Description                                                                 |
| -------                | :-----------                                                                |
| coalesce()             | Returns the first non-null argument                                         |
| endNode()              | Returns the destination node of a relationship                              |
| id()                   | Returns the internal ID of a relationship or node (which is not immutable.) |
| hasLabels()            | Returns true if input node contains all specified labels, otherwise false   |
| labels()               | Returns a string representation of the label of a node                      |
| properties()           | Returns a map containing all the properties in the given map, node, or edge. In case of map, it is returned without any change |
| randomUUID()           | Returns a random UUID (Universal Unique IDentifier)                         |
| startNode()            | Returns the source node of a relationship                                   |
| timestamp()            | Returns the amount of milliseconds since epoch                              |
| type()                 | Returns a string representation of the type of a relationship type          |
| list comprehensions    | [See documentation](#list-comprehensions)                                   |
| pattern comprehensions | [See documentation](#pattern-comprehensions)                                |

## Aggregating functions

|Function         | Description|
| --------------- |:-----------|
|avg()            | Returns the average of a set of numeric values                                               |
|collect()        | Returns a list containing all elements which evaluated from a given expression               |
|count()          | Returns the number of values or rows                                                         |
|max()            | Returns the maximum value in a set of values                                                 |
|min()            | Returns the minimum value in a set of values                                                 |
|sum()            | Returns the sum of a set of numeric values                                                   |
|percentileDisc() | Returns the percentile of the given value over a group, with a percentile from 0.0 to 1.0    |
|percentileCont() | Returns the percentile of the given value over a group, with a percentile from 0.0 to 1.0    |
|stDev()          | Returns the standard deviation for the given value over a group over a sample                |
|stDevP()         | Returns the standard deviation for the given value over a group over an entire population    |

## List functions

| Function            | Description                                                                              |
| ------------------- | :-----------                                                                             |
| head()              | Returns the first member of a list                                                       |
| keys()              | Returns a list of keys containing all properties names in a given map, node, or edge     |
| last()              | Returns the last member of a list                                                        |
| range()             | Creates a new list of integers in the range of [start, end]. If an interval was given, the interval between two consecutive list members will be this interval. |
| size()              | Returns a list size                                                                      |
| tail()              | Returns a sublist of a list, which contains all the values without the first value       |
| [reduce()](#reduce) | Returns a scalar produced by evaluating an expression against each list member           |

## Mathematical operators

|Function     | Description|
| ----------- |:-----------|
| +           | Add two values                                                                                   |
| -           | Subtract second value from first                                                                 |
| *           | Multiply two values                                                                              |
| /           | Divide first value by the second                                                                 |
| ^           | Raise the first value to the power of the second                                                 |
| %           | Perform modulo division of the first value by the second                                         |

## Mathematical functions

|Function     | Description|
| ----------- |:-----------|
| abs()       | Returns the absolute value of a number                                                                                   |
| ceil()      | Returns the smallest floating point number that is greater than or equal to a number and equal to a mathematical integer |
| floor()     | Returns the largest floating point number that is less than or equal to a number and equal to a mathematical integer     |
| rand()      | Returns a random floating point number in the range from 0 to 1; i.e. [0,1]                                              |
| round()     | Returns the value of a number rounded to the nearest integer                                                             |
| sign()      | Returns the signum of a number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number          |
| e()         | Returns the constant _e_, the base of the natural logarithm                                                              |
| exp()       | Returns _e_^a, where _e_ is the base of the natural logarithm                                                            |
| log()       | Returns the natural logarithm of a                                                                                       |
| log10()     | Returns the logarithm (base 10) of a                                                                                     |
| sqrt()      | Returns the square root of a number                                                                                      |
| pow()       | Returns base raised to the power of exponent, base^exponent                                                              |

## Trigonometric functions

|Function    | Description|
| ---------- |:-----------|
| sin()      |  Returns the sine of a numeric expression that represents an angle in radians                            |
| cos()      |  Returns the cosine of a numeric expression that represents an angle in radians                          |
| haversin() |  Returns half the versine of a numeric expression that represents an angle in radians                    |
| tan()      |  Returns the tangent of a numeric expression that represents an angle in radians                         |
| cot()      |  Returns the cotangent of a numeric expression that represents an angle in radians                       |
| asin()     |  Returns the arcsine, in radians, of a numeric expression                                                |
| acos()     |  Returns the arccosine, in radians, of a numeric expression                                              |
| atan()     |  Returns the arctangent, in radians, of a numeric expression                                             |
| atan2()    |  Returns the 2-argument arctangent, in radians, of a pair of numeric expressions (Cartesian coordinates) |
| degrees()  |  Converts a numeric expression from radians to degrees                                                   |
| radians()  |  Converts a numeric expression from degrees to radians                                                   |
| pi()       |  Returns the mathematical constant _pi_                                                                  |

## String functions

| Function    | Description                                                                                     |
| -------     | :-----------                                                                                    |
| left()      | Returns a string containing the specified number of leftmost characters of the original string  |
| lTrim()     | Returns the original string with leading whitespace removed                                     |
| replace()   | Returns a string in which all occurrences of a specified substring are replaced with the specified replacement string |
| reverse()   | Returns a string in which the order of all characters in the original string are reversed       |
| right()     | Returns a string containing the specified number of rightmost characters of the original string |
| rTrim()     | Returns the original string with trailing whitespace removed                                    |
| split()     | Returns a list of strings from splitting a string by a given delimiter                          |
| substring() | Returns a substring of the original string, beginning with a 0-based index start and length     |
| toLower()   | Returns the original string in lowercase                                                        |
| toJSON()    | Returns a [JSON representation](#json-format) of a value                                        |
| toUpper()   | Returns the original string in uppercase                                                        |
| trim()      | Returns the original string with leading and trailing whitespace removed                        |
| size()      | Returns a string length                                                                         |

## Point functions

| Function          | Description                                                     |
| -------           | :-----------                                                    |
| [point()](#point) | Returns a Point type representing the given lat/lon coordinates |
| distance()        | Returns the distance in meters between the two given points     |

## Type conversion functions

|Function    | Description|
| ---------- |:-----------|
| toInteger() | Converts a floating point or string value to an integer value. All other types are not allowed and will cause a type mismatch error to be returned |
| toIntegerOrNull() | Converts a floating point or string value to an integer value. All other types will return `null`                                            | 
| toIntegerList()   | Converts a list of values to a list of integer values. Each item in the list is converted using toIntegerOrNull()      |
| toFloat()   | Converts an integer or string value to a floating point value. All other types are not allowed and will cause a type mismatch error to be returned |
| toFloatOrNull()   | Converts an integer or string value to a floating point value. All other types will return `null`                                            |
| toFloatList()     | Converts a list of values to a list of floating point values. Each item in the list is converted using toFloatOrNull() |
| toString()  | Returns a string representation of a value from the type integer, float, boolean, string, point.All other types are not allowed and will cause a type mismatch error to be returned. |
| toStringorNull()  | Returns a string representation of a value from the type integer, float, boolean, string, point. All other types will return `null` |
| toStringList()    | Converts a list of values to a list of string values. Each item in the list is converted using toStringOrNull()        | 
| toBoolean()       | Returns a boolean value from boolean type, string type (either `"True"` or `"False"` string values, case insensitive. All other values will return `null`) or integer type (0 for `false`, all other values will return `true`). All other types are not allowed and will cause a type mismatch error to be returned.
| toBooleanOrNull() | Returns a boolean value from boolean type, string type (either `"True"` or `"False"` string values, case insensitive. All other values will return `null`) or integer type (0 for `false`, all other values will return `true`). All other types will return `null`.
| toBooleanList()   | Converts a list of values to a list of boolean values. Each item in the list is converted using toBooleanOrNull()      |

## Node functions
|Function | Description|
| ------- |:-----------|
|indegree()  | Returns the number of node's incoming edges |
|outdegree() | Returns the number of node's outgoing edges |

## Path functions
| Function                        | Description                                              |
| -------                         | :-----------                                             |
| nodes()                         | Return a new list of nodes, of a given path              |
| relationships()                 | Return a new list of edges, of a given path              |
| length()                        | Return the length (number of edges) of the path          |
| [shortestPath()](#shortestPath) | Return the shortest path that resolves the given pattern |

### List comprehensions
List comprehensions are a syntactical construct that accepts an array and produces another based on the provided map and filter directives.

They are a common construct in functional languages and modern high-level languages. In Cypher, they use the syntax:

```sh
[element IN array WHERE condition | output elem]
```

- `array` can be any expression that produces an array: a literal, a property reference, or a function call.
- `WHERE condition` is an optional argument to only project elements that pass a certain criteria. If omitted, all elements in the array will be represented in the output.
- `| output elem` is an optional argument that allows elements to be transformed in the output array. If omitted, the output elements will be the same as their corresponding inputs.


The following query collects all paths of any length, then for each produces an array containing the `name` property of every node with a `rank` property greater than 10:

```sh
MATCH p=()-[*]->() RETURN [node IN nodes(p) WHERE node.rank > 10 | node.name]
```

#### Existential comprehension functions
The functions `any()`, `all()`, `single()` and `none()` use a simplified form of the list comprehension syntax and return a boolean value.

```sh
any(element IN array WHERE condition)
```

They can operate on any form of input array, but are particularly useful for path filtering. The following query collects all paths of any length in which all traversed edges have a weight less than 3:

```sh
MATCH p=()-[*]->() WHERE all(edge IN relationships(p) WHERE edge.weight < 3) RETURN p
```

### Pattern comprehensions

Pattern comprehensions are a method of producing a list composed of values found by performing the traversal of a given graph pattern.

The following query returns the name of a `Person` node and a list of all their friends' ages:

```sh
MATCH (n:Person)
RETURN
n.name,
[(n)-[:FRIEND_OF]->(f:Person) | f.age]
```

Optionally, a `WHERE` clause may be embedded in the pattern comprehension to filter results. In this query, all friends' ages will be gathered for friendships that started before 2010:

```sh
MATCH (n:Person)
RETURN
n.name,
[(n)-[e:FRIEND_OF]->(f:Person) WHERE e.since < 2010 | f.age]
```

### CASE WHEN

The case statement comes in two variants. Both accept an input argument and evaluates it against one or more expressions. The first `WHEN` argument that specifies a value matching the result will be accepted, and the value specified by the corresponding `THEN` keyword will be returned.

Optionally, an `ELSE` argument may also be specified to indicate what to do if none of the `WHEN` arguments match successfully.

In its simple form, there is only one expression to evaluate and it immediately follows the `CASE` keyword:

```sh
MATCH (n)
RETURN
CASE n.title
WHEN 'Engineer' THEN 100
WHEN 'Scientist' THEN 80
ELSE n.privileges
END
```

In its generic form, no expression follows the `CASE` keyword. Instead, each `WHEN` statement specifies its own expression:

```sh
MATCH (n)
RETURN
CASE
WHEN n.age < 18 THEN '0-18'
WHEN n.age < 30 THEN '18-30'
ELSE '30+'
END
```

#### Reduce
The `reduce()` function accepts a starting value and updates it by evaluating an expression against each element of the list:

```sh
RETURN reduce(sum = 0, n IN [1,2,3] | sum + n)
```

`sum` will successively have the values 0, 1, 3, and 6, with 6 being the output of the function call.

### Point
The `point()` function expects one map argument of the form:
```sh
RETURN point({latitude: lat_value, longitude: lon_val})
```

The key names `latitude` and `longitude` are case-sensitive.

The point constructed by this function can be saved as a node/relationship property or used within the query, such as in a `distance` function call.

### shortestPath
The `shortestPath()` function is invoked with the form:
```sh
MATCH (a {v: 1}), (b {v: 4}) RETURN shortestPath((a)-[:L*]->(b))
```

The sole `shortestPath` argument is a traversal pattern. This pattern's endpoints must be resolved prior to the function call, and no property filters may be introduced in the pattern. The relationship pattern may specify any number of relationship types (including zero) to be considered. If a minimum number of edges to traverse is specified, it may only be 0 or 1, while any number may be used for the maximum. If 0 is specified as the minimum, the source node will be included in the returned path. If no shortest path can be found, NULL is returned.

### JSON format
`toJSON()` returns the input value in JSON formatting. For primitive data types and arrays, this conversion is conventional. Maps and map projections (`toJSON(node { .prop} )`) are converted to JSON objects, as are nodes and relationships.

The format for a node object in JSON is:
```sh
{
  "type": "node",
  "id": id(int),
  "labels": [label(string) X N],
  "properties": {
    property_key(string): property_value X N
  }
}
```

The format for a relationship object in JSON is:
```sh
{
  "type": "relationship",
  "id": id(int),
  "relationship": type(string),
  "properties": {
    property_key(string): property_value X N
  }
  "start": src_node(node),
  "end": dest_node(node)
}
```

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

| Procedure                       | Arguments                                       | Yields                        | Description                                                                                                                                                                            |
| -------                         | :-------                                        | :-------                      | :-----------                                                                                                                                                                           |
| db.labels                       | none                                            | `label`                       | Yields all node labels in the graph.                                                                                                                                                   |
| db.relationshipTypes            | none                                            | `relationshipType`            | Yields all relationship types in the graph.                                                                                                                                            |
| db.propertyKeys                 | none                                            | `propertyKey`                 | Yields all property keys in the graph.                                                                                                                                                 |
| db.indexes                      | none                                            | `type`, `label`, `properties`, `language`, `stopwords`, `entityType`, `info` | Yield all indexes in the graph, denoting whether they are exact-match or full-text and which label and properties each covers and whether they are indexing node or relationship attributes.                                                         |
| db.idx.fulltext.createNodeIndex | `label`, `property` [, `property` ...]          | none                          | Builds a full-text searchable index on a label and the 1 or more specified properties.                                                                                                 |
| db.idx.fulltext.drop            | `label`                                         | none                          | Deletes the full-text index associated with the given label.                                                                                                                           |
| db.idx.fulltext.queryNodes      | `label`, `string`                               | `node`, `score`               | Retrieve all nodes that contain the specified string in the full-text indexes on the given label.                                                                                      |
| algo.pageRank                   | `label`, `relationship-type`                    | `node`, `score`               | Runs the pagerank algorithm over nodes of given label, considering only edges of given relationship type.                                                                              |
| [algo.BFS](#BFS)                | `source-node`, `max-level`, `relationship-type` | `nodes`, `edges`              | Performs BFS to find all nodes connected to the source. A `max level` of 0 indicates unlimited and a non-NULL `relationship-type` defines the relationship type that may be traversed. |
| dbms.procedures()               | none                                            | `name`, `mode`                | List all procedures in the DBMS, yields for every procedure its name and mode (read/write).                                                                                            |

### Algorithms

#### BFS
The breadth-first-search algorithm accepts 4 arguments:

`source-node (node)` - The root of the search.

`max-level (integer)` - If greater than zero, this argument indicates how many levels should be traversed by BFS. 1 would retrieve only the source's neighbors, 2 would retrieve all nodes within 2 hops, and so on.

`relationship-type (string)` - If this argument is NULL, all relationship types will be traversed. Otherwise, it specifies a single relationship type to perform BFS over.

It can yield two outputs:

`nodes` - An array of all nodes connected to the source without violating the input constraints.

`edges` - An array of all edges traversed during the search. This does not necessarily contain all edges connecting nodes in the tree, as cycles or multiple edges connecting the same source and destination do not have a bearing on the reachability this algorithm tests for. These can be used to construct the directed acyclic graph that represents the BFS tree. Emitting edges incurs a small performance penalty.

## Indexing

RedisGraph supports single-property indexes for node labels and for relationship type. String, numeric, and geospatial data types can be indexed.

### Creating an index for a node label

For a node label, the index creation syntax is:

```sh
GRAPH.QUERY DEMO_GRAPH "CREATE INDEX FOR (p:Person) ON (p.age)"
```

An old syntax is also supported:

```sh
GRAPH.QUERY DEMO_GRAPH "CREATE INDEX ON :Person(age)"
```

After an index is explicitly created, it will automatically be used by queries that reference that label and any indexed property in a filter.

```sh
GRAPH.EXPLAIN DEMO_GRAPH "MATCH (p:Person) WHERE p.age > 80 RETURN p"
1) "Results"
2) "    Project"
3) "        Index Scan | (p:Person)"
```

This can significantly improve the runtime of queries with very specific filters. An index on `:employer(name)`, for example, will dramatically benefit the query:

```sh
GRAPH.QUERY DEMO_GRAPH
"MATCH (:Employer {name: 'Dunder Mifflin'})-[:EMPLOYS]->(p:Person) RETURN p"
```

An example of utilizing a geospatial index to find `Employer` nodes within 5 kilometers of Scranton is:

```sh
GRAPH.QUERY DEMO_GRAPH
"WITH point({latitude:41.4045886, longitude:-75.6969532}) AS scranton MATCH (e:Employer) WHERE distance(e.location, scranton) < 5000 RETURN e"
```

Geospatial indexes can currently only be leveraged with `<` and `<=` filters; matching nodes outside of the given radius is performed using conventional matching.

### Creating an index for a relationship type

For a relationship type, the index creation syntax is:

```sh
GRAPH.QUERY DEMO_GRAPH "CREATE INDEX FOR ()-[f:FOLLOW]-() ON (f.created_at)"
```

Then the execution plan for using the index:

```sh
GRAPH.EXPLAIN DEMO_GRAPH "MATCH (p:Person {id: 0})-[f:FOLLOW]->(fp) WHERE 0 < f.created_at AND f.created_at < 1000 RETURN fp"
1) "Results"
2) "    Project"
3) "        Edge By Index Scan | [f:FOLLOW]"
4) "            Node By Index Scan | (p:Person)"
```

This can significantly improve the runtime of queries that traverse super nodes or when we want to start traverse from relationships.

### Deleting an index for a node label

For a node label, the index deletion syntax is:

```sh
GRAPH.QUERY DEMO_GRAPH "DROP INDEX ON :Person(age)"
```

### Deleting an index for a relationship type

For a relationship type, the index deletion syntax is:

```sh
GRAPH.QUERY DEMO_GRAPH "DROP INDEX ON :FOLLOW(created_at)"
```

## Full-text indexing

RedisGraph leverages the indexing capabilities of [RediSearch](/docs/stack/search/index.html) to provide full-text indices through procedure calls. 

### Creating a full-text index for a node label

To construct a full-text index on the `title` property of all nodes with label `Movie`, use the syntax:

```sh
GRAPH.QUERY DEMO_GRAPH "CALL db.idx.fulltext.createNodeIndex('Movie', 'title')"
```

More properties can be added to this index by adding their names to the above set of arguments, or using this syntax again with the additional names.

```sh
GRAPH.QUERY DEMO_GRAPH "CALL db.idx.fulltext.createNodeIndex('Person', 'firstName', 'lastName')"
```

RediSearch provide 2 index configuration options:
1. Language - Define which language to use for stemming text which is adding the base form of a word to the index. This allows the query for "going" to also return results for "go" and "gone", for example.
2. Stopwords - These are words that are usually so common that they do not add much information to search, but take up a lot of space and CPU time in the index.

To construct a full-text index on the `title` property using `German` language and using custom stopwords of all nodes with label `Movie`, use the syntax:

```sh
GRAPH.QUERY DEMO_GRAPH "CALL db.idx.fulltext.createNodeIndex({ label: 'Movie', language: 'German', stopwords: ['a', 'ab'] }, 'title')"
```

RediSearch provide 3 additional field configuration options:
1. Weight - The importance of the text in the field
2. Nostem - Skip stemming when indexing text
3. Phonetic - Enable phonetic search on the text

To construct a full-text index on the `title` property with phonetic search of all nodes with label `Movie`, use the syntax:

```sh
GRAPH.QUERY DEMO_GRAPH "CALL db.idx.fulltext.createNodeIndex('Movie', {field: 'title', phonetic: 'dm:en'})"
```

### Utilizing a full-text index for a node label

An index can be invoked to match any whole words contained within:

```sh
GRAPH.QUERY DEMO_GRAPH
"CALL db.idx.fulltext.queryNodes('Movie', 'Book') YIELD node RETURN node.title"
1) 1) "node.title"
2) 1) 1) "The Jungle Book"
   2) 1) "The Book of Life"
3) 1) "Query internal execution time: 0.927409 milliseconds"
```

This CALL clause can be interleaved with other Cypher clauses to perform more elaborate manipulations:
```sh
GRAPH.QUERY DEMO_GRAPH
"CALL db.idx.fulltext.queryNodes('Movie', 'Book') YIELD node AS m
WHERE m.genre = 'Adventure'
RETURN m ORDER BY m.rating"
1) 1) "m"
2) 1) 1) 1) 1) "id"
            2) (integer) 1168
         2) 1) "labels"
            2) 1) "Movie"
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

In addition to yielding matching nodes, full-text index scans will return the score of each node. This is the [TF-IDF](/docs/stack/search/reference/scoring/#tfidf-default) score of the node, which is informed by how many times the search terms appear in the node and how closely grouped they are. This can be observed in the example:
```sh
GRAPH.QUERY DEMO_GRAPH
"CALL db.idx.fulltext.queryNodes('Node', 'hello world') YIELD node, score RETURN score, node.val"
1) 1) "score"
   2) "node.val"
2) 1) 1) "2"
      2) "hello world"
   2) 1) "1"
      2) "hello to a different world"
3) 1) "Cached execution: 1"
   2) "Query internal execution time: 0.335401 milliseconds"
```

### Deleting a full-text index for a node label

For a node label, the full-text index deletion syntax is:

```
GRAPH.QUERY DEMO_GRAPH "CALL db.idx.fulltext.drop('Movie')"
```

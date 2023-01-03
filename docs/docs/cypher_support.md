---
title: "Cypher coverage"
linkTitle: "Cypher coverage"
weight: 7
description: >
    RedisGraph implements a subset of the Cypher language, which is growing as development continues.
    This document is based on the Cypher Query Language Reference (version 9), available at [OpenCypher Resources](https://www.opencypher.org/resources).
---

## Patterns
Patterns are fully supported.

## Types
### Structural types
+ Nodes
+ Relationships
+ Path variables (alternating sequence of nodes and relationships).


### Composite types
+ Lists
+ Maps

  **Unsupported:**

- Temporal types (Date, DateTime, LocalDateTime, Time, LocalTime, Duration)

### Literal types
+ Numeric types (64-bit doubles and 64-bit signed integer representations)
+ String literals
+ Booleans

  **Unsupported:**

- Hexadecimal and octal numerics

### Other
NULL is supported as a representation of a missing or undefined value.

## Comparability, equality, orderability, and equivalence
This is a somewhat nebulous area in Cypher itself, with a lot of edge cases.
Broadly speaking, RedisGraph behaves as expected with string and numeric values.
There are likely some behaviors involving the numerics NaN, -inf, inf, and possibly -0.0 that deviate from the Cypher standard.
We do not support any of these properties at the type level, meaning nodes and relationships are not internally comparable.

## Clauses
### Reading Clauses
+ MATCH
+ OPTIONAL MATCH

### Projecting Clauses
+ RETURN
+ AS
+ WITH
+ UNWIND

### Reading sub-clauses
+ WHERE
+ ORDER BY
+ SKIP
+ LIMIT

### Writing Clauses
+ CREATE
+ DELETE
    + We actually implement DETACH DELETE, the distinction being that relationships invalidated by node deletions are automatically deleted.
+ SET

  **Unsupported:**

- REMOVE (to modify properties)
    + Properties can be deleted with SET [prop] = NULL.

### Reading/Writing Clauses
+ MERGE
+ CALL (procedures)
    - The currently-supported procedures are listed in [the Procedures documentation](/commands/graph.query/#procedures).

### Set Operations
+ UNION
+ UNION ALL

## Functions

The currently-supported functions are listed in [the Functions documentation](/commands/graph.query/#functions).

  **Unsupported:**

- Temporal arithmetic functions
- User-defined functions

## Operators

### Mathematical operators

The currently-supported functions are listed in [the mathematical operators documentation](/commands/graph.query/#mathematical-operators).

### String operators

+ String operators (STARTS WITH, ENDS WITH, CONTAINS) are supported.

  **Unsupported:**

- Regex operator


### Boolean operators
+ AND
+ OR
+ NOT
+ XOR

## Parameters
Parameters may be specified to allow for more flexible query construction:
```sh
CYPHER name_param = "Niccolò Machiavelli" birth_year_param = 1469 MATCH (p:Person {name: $name_param, birth_year: $birth_year_param}) RETURN p
```
The example above shows the syntax used by `redis-cli` to set parameters, but 
each RedisGraph client introduces a language-appropriate method for setting parameters,
and is described in their documentation.

## Non-Cypher queries
+ RedisGraph provides the `GRAPH.EXPLAIN` command to print the execution plan of a provided query.
+ `GRAPH.DELETE` will remove a graph and all Redis keys associated with it.
- We do not currently provide support for queries that retrieve schemas, though the LABELS and TYPE scalar functions may be used to get a graph overview.

# Cypher Coverage
RedisGraph implements a subset of the Cypher language, which is growing as development continues.
This document is based on the Cypher Query Language Reference (version 9), available at [OpenCypher Resources](https://www.opencypher.org/resources).

## Patterns
Patterns are fully supported.

## Types
### Structural types
+ Nodes
    - Nodes are fully supported save that a node can only be associated with a single label.
+ Relationships
    - Undirected relationships are not supported.

  **Unsupported:**

- Path variables (alternating sequence of nodes and relationships)

### Composite types
+ Lists

  **Unsupported:**

- Maps
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

  **Unsupported:**

- OPTIONAL MATCH
- MANDATORY MATCH

### Projecting Clauses
+ RETURN
+ AS
+ WITH
+ UNWIND

### Reading sub-clauses
+ WHERE
+ ORDER BY
    - ASC and DESC are supported at the level of the sub-clause, but not for individual properties (issue #96)
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
    - The currently-supported procedures can be found in [the Procedures documentation](commands.md#procedures).

### Set Operations
**Unsupported:**

- UNION / UNION ALL

## Functions
### Scalar functions
+ id
+ labels
+ timestamp
+ type

  **Unsupported:**

- coalesce
- Casting functions (toBoolean, toFloat, toInteger)
- Relationship functions (startNode, endNode, type)
- Temporal arithmetics 

### Aggregating functions
+ avg
+ collect
+ count
+ max
+ min
+ percentileCont
+ percentileDist
+ stDev
+ stDevP
+ sum

### List functions
+ head 
+ range
+ reverse
+ size
+ tail 

### Math functions - numeric
+ abs
+ ceil
+ floor
+ sign
+ round
+ rand

### String functions
+ left
+ right
+ trim
+ lTrim
+ rTrim
+ reverse
+ substring
+ toLower
+ toUpper
+ toString

  **Unsupported:**

- replace
- split

### Unsupported function classes

- Logarithmic math functions
- Trigonometric math functions
- User-defined functions
- Predicate functions (the only function in this class is EXISTS)


## Operators

### Mathematical operators
+ Multiplication, addition, subtraction, division

  **Unsupported:**

- Modulo, exponentiation

### String operators
String operators (STARTS WITH, ENDS WITH, CONTAINS) are supported.

### Boolean operators
+ AND
+ OR
+ NOT
+ XOR

### Other
CASE expressions.

## Non-Cypher queries
+ RedisGraph provides the `GRAPH.EXPLAIN` command to print the execution plan of a provided query.
+ `GRAPH.DELETE` will remove a graph and all Redis keys associated with it.
- We do not currently provide support for queries that retrieve schemas, though the LABELS and TYPE scalar functions may be used to get a graph overview.

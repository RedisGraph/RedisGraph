# Cypher Coverage
RedisGraph implements a subset of the Cypher language, which is growing as development continues.
This document is based on comparison to the Cypher Query Language Reference (version 9), available at [OpenCypher Resources](https://www.opencypher.org/resources).

## Patterns
Patterns are fully supported with the exception of the OR (`[:rel_a|:rel_b]`) operator to explicitly specify multiple relationship types.

## Types
### Structural types
+ Nodes
    - Nodes are fully supported save that a node can only be associated with a single label.
+ Relationships
    - Undirected relationships are not supported.

  **Unsupported:**

- Path variables (alternating sequence of nodes and relationships)

### Composite types
  **Unsupported:**

- Lists
- Maps

### Literal types
+ Numeric types (though in most cases, we convert integers to 64-bit double representations)
+ String literals

  **Unsupported:**

- Hexadecimal and octal numerics
- Booleans

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

  **Unsupported:**

- WITH
- UNWIND

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

### Reading/Writing Clauses
+ MERGE
    - MERGE is only supported as a standalone clause, and as such cannot be combined with other directives such as MATCH or RETURN.
  **Unsupported:**

- CALL (stored procedures)

### Set Operations
**Unsupported:**

- UNION / UNION ALL

## Functions
### Scalar functions
+ id

  **Unsupported:**

- coalesce
- timestamp
- Casting functions (toBoolean, toFloat, toInteger)
- Relationship functions (startNode, endNode, type)
- List functions (head, last, length, size)

### Aggregating functions
+ avg
+ count
+ max
+ min
+ percentileCont
+ percentileDist
+ stDev
+ stDevP
+ sum

  **Unsupported:**

- collect

### Math functions - numeric
+ abs
+ ceil
+ floor
+ sign

  **Unsupported:**

- rand
- round

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

- Logarithmetic math functions
- Trigonometric math functions
- List functions
- User-defined functions
- Predicate functions (the only function in this class is EXISTS)


## Operators

### Mathematical operators
+ Multiplication, addition, subtraction, division

  **Unsupported:**

- Modulo, exponentiation

### String operators
String operators (STARTS WITH, ENDS WITH, CONTAINS) are not supported, though equivalent results can be obtained using string functions.

### Boolean operators
+ AND
+ OR
+ NOT (currently implemented with the syntax `!=`)

  **Unsupported:**

- XOR

### Other
CASE operators are not supported.

## Non-Cypher queries
+ RedisGraph provides the `GRAPH.EXPLAIN` command to print the execution plan of a provided query.
+ `GRAPH.DELETE` will remove a graph and all Redis keys associated with it.
- We do not currently provide support for queries that retrieve schemas.

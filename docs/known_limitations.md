# Known limitations

## Relationship uniqueness in patterns

When a relation in a match pattern is not referenced elsewhere in the query, RedisGraph will only verify that at least one matching relation exists (rather than operating on every matching relation).

In some queries, this will cause unexpected behaviors. Consider a graph with 2 nodes and 2 relations between them:

```sh
CREATE (a)-[:e {val: '1'}]->(b), (a)-[:e {val: '2'}]->(b)
```

Counting the number of explicit edges returns 2, as expected.

```sh
MATCH (a)-[e]->(b) RETURN COUNT(e)
```

However, if we count the nodes in this pattern without explicitly referencing the relation, we receive a value of 1.

```sh
MATCH (a)-[e]->(b) RETURN COUNT(b)
```

We are researching designs that resolve this problem without negatively impacting performance. As a temporary workaround, queries that must operate on every relation matching a pattern should explicitly refer to that relation's alias elsewhere in the query. Two options for this are:

```sh
MATCH (a)-[e]->(b) WHERE ID(e) >= 0 RETURN COUNT(b)
MATCH (a)-[e]->(b) RETURN COUNT(b), e.dummyval
```

## WITH clause limitations

### Unaliased WITH entities

```sh
MATCH (x) WITH x RETURN x
```

Will return a syntax error - use a construction like `WITH x AS y` instead.

### Returning full nodes and relationships specified in WITH clause

```sh
MATCH (x) WITH x AS y RETURN y
```

Every row in the result set will be a null value.

Specified entities may be modified or have explicitly named properties returned, as in:

```sh
MATCH (x) WITH x AS y RETURN y.name"
```

### Using nodes and relationships specified in WITH clause in multiple patterns

```sh
MATCH (x {val: 1}) WITH x AS src MATCH (src)-[]->(dest) RETURN dest
```

Will return a validation error.

### Reusing identifiers in separate clauses

```sh
MATCH (a) WITH a.val AS val MATCH ()-[a]->() RETURN a
```

Will return a validation error.

### Specifying WITH entities in curly braces:

```sh
UNWIND [1,2,3] AS a WITH a AS value CREATE (:label_a {val: value})
UNWIND [1,2,3] AS a WITH a AS value MATCH (a {val: value})
```

Will return a syntax error.

WITH entities may be used in other contexts, such as:

```sh
UNWIND [1,2,3] AS a WITH a AS value MATCH (b) WHERE b.val = value RETURN b
```

### Specifying DISTINCT on a non-return clause

```sh
MATCH (a) WITH DISTINCT a.val AS distinct_val RETURN distinct_val
```

Will return a syntax error.

## CREATE clause limitations

### CREATE ... RETURN

```sh
CREATE (a {val: 1}) RETURN a
```

Will return a syntax error, as entities that are constructed from a CREATE clause cannot currently be returned in the same query.

This limitation does not apply to queries of the form MATCH ... CREATE ... RETURN.

## Indexing limitations

One way in which RedisGraph will optimize queries is by introducing index scans when a filter is specified on an indexed label-property pair.

The current index implementation, however, does not handle `!=` (NOT) and `OR` filters.

To profile a query and see whether index optimizations have been introduced, use the `GRAPH.EXPLAIN` endpoint:

```sh
$ redis-cli GRAPH.EXPLAIN social "MATCH (p:person) WHERE p.id < 5 RETURN p"
"Results\n    Project\n        Index Scan\n"
```


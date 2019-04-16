# Known limitations

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

### Relation uniqueness in MATCH ... CREATE

CREATE executes for each pair of connected nodes, rather than once for each unique relation connecting two nodes.

If we have built a graph with the command:

```sh
CREATE (a)-[:e {val: '1'}]->(b), (a)-[:e {val: '2'}]->(b)
```

We have 2 nodes and 2 relations between them.

```sh
MATCH (a)-[]->(b) CREATE (a)-[:relclone]->(b)
```

Will only create 1 relationship.

If CREATE should be triggered once for each relation, the relation should be referenced. Both of the following commands would create 2 relations:

```sh
MATCH (a)-[e]->(b) CREATE (a)-[:relclone]->(b) RETURN e
MATCH (a)-[e]->(b) WHERE ID(e) >= 0  CREATE (a)-[:relclone]->(b)
```

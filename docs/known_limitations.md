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

# Technical specification for writing RedisGraph client libraries

By design, there is not a full standard for RedisGraph clients to adhere to. Areas such as pretty-print formatting, query validation, and transactional and multithreaded capabilities have no canonically correct behavior, and the implementer is free to choose the approach and complexity that suits them best.

RedisGraph does, however, provide a compact result set format for clients that minimizes the amount of redundant data transmitted from the server. Implementers are encouraged to take advantage of this format, as it provides better performance and removes ambiguity from decoding certain data. This approach requires clients to be capable of issuing procedure calls to the server and performing a small amount of client-side caching.

## Retrieving the compact result set

Appending the flag `--compact` to any query issued to the GRAPH.QUERY endpoint will cause the server to issue results in the compact format. Because we don't store connection-specific configurations, all queries should be issued with this flag.
```sh
GRAPH.QUERY demo "MATCH (a) RETURN a" --compact
```

## Formatting differences in the compact result set

The result set has the same overall structure as described in the [Result Set documentation](result_structure.md#top-level-members).

Certain values are emitted as integer IDs rather than strings:

1. Node labels
2. Relationship types
3. Property keys

Instructions on how to efficiently convert these IDs in the [Procedure Calls](#procedure-calls) section below.

Additionally, two enums are exposed:

[ColumnType](https://github.com/RedisGraph/RedisGraph/blob/ff108d7e21061025166a35d29be1a1cb5bac6d55/src/resultset/formatters/resultset_formatter.h#L14-L19), which as of RedisGraph v2.1.0 will always be `COLUMN_SCALAR`. This enum is retained for backwards compatibility, and may be ignored by the client unless RedisGraph versions older than v2.1.0 must be supported.

[ValueType](https://github.com/RedisGraph/RedisGraph/blob/ff108d7e21061025166a35d29be1a1cb5bac6d55/src/resultset/formatters/resultset_formatter.h#L21-L28) indicates the data type (such as Node, integer, or string) of each returned value. Each value is emitted as a 2-array, with this enum in the first position and the actual value in the second. Each property on a graph entity also has a scalar as its value, so this construction is nested in each value of the properties array when a column contains a node or relationship.

## Decoding the result set

Given the graph created by the query:
```sh
GRAPH.QUERY demo "CREATE (:plant {name: 'Tree'})-[:GROWS {season: 'Autumn'}]->(:fruit {name: 'Apple'})"
```

Let's formulate a query that returns 3 columns: nodes, relationships, and scalars, in that order.

Verbose (default):
```sh
127.0.0.1:6379> GRAPH.QUERY demo "MATCH (a)-[e]->(b) RETURN a, e, b.name"
1) 1) "a"
   2) "e"
   3) "b.name"
2) 1) 1) 1) 1) "id"
            2) (integer) 0
         2) 1) "labels"
            2) 1) "plant"
         3) 1) "properties"
            2) 1) 1) "name"
                  2) "Tree"
      2) 1) 1) "id"
            2) (integer) 0
         2) 1) "type"
            2) "GROWS"
         3) 1) "src_node"
            2) (integer) 0
         4) 1) "dest_node"
            2) (integer) 1
         5) 1) "properties"
            2) 1) 1) "season"
                  2) "Autumn"
      3) "Apple"
3) 1) "Query internal execution time: 1.326905 milliseconds"
```

Compact:
```sh
127.0.0.1:6379> GRAPH.QUERY demo "MATCH (a)-[e]->(b) RETURN a, e, b.name" --compact
1) 1) 1) (integer) 1
      2) "a"
   2) 1) (integer) 1
      2) "e"
   3) 1) (integer) 1
      2) "b.name"
2) 1) 1) 1) (integer) 8
         2) 1) (integer) 0
            2) 1) (integer) 0
            3) 1) 1) (integer) 0
                  2) (integer) 2
                  3) "Tree"
      2) 1) (integer) 7
         2) 1) (integer) 0
            2) (integer) 0
            3) (integer) 0
            4) (integer) 1
            5) 1) 1) (integer) 1
                  2) (integer) 2
                  3) "Autumn"
      3) 1) (integer) 2
         2) "Apple"
3) 1) "Query internal execution time: 1.085412 milliseconds"
```

These results are being parsed by `redis-cli`, which adds such visual cues as array indexing and indentation, as well as type hints like `(integer)`. The actual data transmitted is formatted using the [RESP protocol](https://redis.io/topics/protocol). All of the current RedisGraph clients rely upon a stable Redis client in the same language (such as [redis-rb](https://github.com/redis/redis-rb) for Ruby) which handles RESP decoding. 

### Top-level array results

The result set above had 3 members in its top-level array:
```sh
1) Header row
2) Result rows
3) Query statistics
```

All queries that have a `RETURN` clause will have these 3 members. Queries that don't return results have only one member in the outermost array, the query statistics:
```sh
127.0.0.1:6379> GRAPH.QUERY demo "CREATE (:plant {name: 'Tree'})-[:GROWS {season: 'Autumn'}]->(:fruit {name: 'Apple'})" --compact
1) 1) "Labels added: 2"
   2) "Nodes created: 2"
   3) "Properties set: 3"
   4) "Relationships created: 1"
   5) "Query internal execution time: 1.972868 milliseconds"
```

Rather than introspecting on the query being emitted, the client implementation can check whether this array contains 1 or 3 elements to choose how to format data. 

### Reading the header row

Our sample query `MATCH (a)-[e]->(b) RETURN a, e, b.name` generated the header:
```sh
1) 1) (integer) 1
   2) "a"
3) 1) (integer) 1
   3) "e"
4) 1) (integer) 1
   3) "b.name"
```

The 4 array members correspond, in order, to the 3 entities described in the RETURN clause.

Each is emitted as a 2-array:
```sh
1) ColumnType (enum)
2) column name (string)
```

The first element is the [ColumnType enum](https://github.com/RedisGraph/RedisGraph/blob/master/src/resultset/formatters/resultset_formatter.h#L14-L19), which as of RedisGraph v2.1.0 will always be `COLUMN_SCALAR`. This element is retained for backwards compatibility, and may be ignored by the client unless RedisGraph versions older than v2.1.0 must be supported.

### Reading result rows

The entity representations in this section will closely resemble those found in [Result Set Graph Entities](result_structure.md#graph-entities).

Our query produced one row of results with 3 columns (as described by the header):
```sh
1) 1) 1) (integer) 8
      2) 1) (integer) 0
         2) 1) (integer) 0
         3) 1) 1) (integer) 0
               2) (integer) 2
               3) "Tree"
   2) 1) (integer) 7
      2) 1) (integer) 0
         2) (integer) 0
         3) (integer) 0
         4) (integer) 1
         5) 1) 1) (integer) 1
               2) (integer) 2
               3) "Autumn"
   3) 1) (integer) 2
      2) "Apple"
```
Each element is emitted as a 2-array - [`ValueType`, value].

It is the client's responsibility to store the [ValueType enum](https://github.com/RedisGraph/RedisGraph/blob/master/src/resultset/formatters/resultset_formatter.h#L21-L28). RedisGraph guarantees that this enum may be extended in the future, but the existing values will not be altered.

The `ValueType` for the first entry is `VALUE_NODE`. The node representation contains 3 top-level elements:

1. The node's internal ID.
2. An array of all label IDs associated with the node (currently, each node can have either 0 or 1 labels, though this restriction may be lifted in the future).
3. An array of all properties the node contains. Properties are represented as 3-arrays - [property key ID, `ValueType`, value].

```sh
[	
    Node ID (integer),
    [label ID (integer) X label count]
    [[property key ID (integer), ValueType (enum), value (scalar)] X property count]
]
```

The `ValueType` for the first entry is `VALUE_EDGE`. The edge representation differs from the node representation in two respects:

- Each relation has exactly one type, rather than the 0+ labels a node may have.
- A relation is emitted with the IDs of its source and destination nodes.

As such, the complete representation is as follows:

1. The relation's internal ID.
2. The relationship type ID.
3. The source node's internal ID.
4. The destination node's internal ID.
5. The key-value pairs of all properties the relation possesses.

```sh
[	
    Relation ID (integer),
    type ID (integer),
    source node ID (integer),
    destination node ID (integer),
    [[property key ID (integer), ValueType (enum), value (scalar)] X property count]
]
```

The `ValueType` for the third entry is `VALUE_STRING`, and the other element in the array is the actual value, "Apple".

### Reading statistics

The final top-level member of the GRAPH.QUERY reply is the execution statistics. This element is identical between the compact and standard response formats.

The statistics always include query execution time, while any combination of the other elements may be included depending on how the graph was modified.

1. "Labels added: (integer)"
2. "Nodes created: (integer)"
3. "Properties set: (integer)"
4. "Nodes deleted: (integer)"
5. "Relationships deleted: (integer)"
6. "Relationships created: (integer)"
7. "Query internal execution time: (float) milliseconds"

## Procedure Calls

Property keys, node labels, and relationship types are all returned as IDs rather than strings in the compact format. For each of these 3 string-ID mappings, IDs start at 0 and increase monotonically.

As such, the client should store an string array for each of these 3 mappings, and print the appropriate string for the user by checking an array at position _ID_. If an ID greater than the array length is encountered, the local array should be updated with a procedure call.

These calls are described generally in the [Procedures documentation](commands.md#procedures).

To retrieve each full mapping, the appropriate calls are:

`db.labels()`
```sh
127.0.0.1:6379> GRAPH.QUERY demo "CALL db.labels()"
1) 1) "label"
2) 1) 1) "plant"
   2) 1) "fruit"
3) 1) "Query internal execution time: 0.321513 milliseconds"
```

`db.relationshipTypes()`
```sh
127.0.0.1:6379> GRAPH.QUERY demo "CALL db.relationshipTypes()"
1) 1) "relationshipType"
2) 1) 1) "GROWS"
3) 1) "Query internal execution time: 0.429677 milliseconds"
```

`db.propertyKeys()`
```sh
127.0.0.1:6379> GRAPH.QUERY demo "CALL db.propertyKeys()"
1) 1) "propertyKey"
2) 1) 1) "name"
   2) 1) "season"
3) 1) "Query internal execution time: 0.318940 milliseconds"
```

Because the cached values never become outdated, it is possible to just retrieve new values with slightly more complex constructions:
```sh
CALL db.propertyKeys() YIELD propertyKey RETURN propertyKey SKIP [cached_array_length]
```
Though the property calls are quite efficient regardless of whether this optimization is used.

As an example, the Python client checks its local array of labels to resolve every label ID [as seen here](https://github.com/RedisGraph/redisgraph-py/blob/d65ec325b1909489845427b7100dcba6c4050b66/redisgraph/graph.py#L20-L32).

In the case of an IndexError, it issues a procedure call to fully refresh its label cache [as seen here](https://github.com/RedisGraph/redisgraph-py/blob/d65ec325b1909489845427b7100dcba6c4050b66/redisgraph/graph.py#L153-L154).

## Reference clients

All the logic described in this document has been implemented in most of the clients listed in [Client Libraries](clients.md#currently-available-libraries). Among these, `redisgraph-py` and `JRedisGraph` are currently the most sophisticated.


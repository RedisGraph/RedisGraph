# RedisGraph Result Set Structure

This document describes the format RedisGraph uses to print data when accessed through the `redis-cli` utility.

The [language-specific clients](clients.md) retrieve data in a more succinct format, and provide their own functionality for printing result sets.

## Top-level members
Queries that return data emit an array with 3 top-level members:

1. The header describing the returned records. This is an array which corresponds precisely in order and naming to the RETURN clause of the query.
2. A nested array containing the actual data returned by the query.
3. An array of metadata related to the query execution. This includes query runtime as well as data changes, such as the number of entities created, deleted, or modified by the query.

## Result Set data types

A column in the result set can be populated with graph entities (nodes or relations) or scalar values.

### Scalars

RedisGraph replies are formatted using the [RESP protocol](https://redis.io/topics/protocol). The current RESP iteration provides fewer data types than RedisGraph supports internally, so displayed results are mapped as follows:

| RedisGraph type | Display format              |
|-----------------|-----------------------------|
| Integer         | Integer                     |
| NULL            | NULL (nil)                  |
| String          | String                      |
| Boolean         | String ("true"/"false")     |
| Double          | String (15-digit precision) |


### Graph Entities

When full entities are specified in a RETURN clause, all data relevant to each entity value is emitted within a nested array. Key-value pairs in the data, such as the combination of a property name and its corresponding value, are represented as 2-arrays.

Internal IDs are returned with nodes and relations, but these IDs are not immutable. After entities have been deleted, higher IDs may be migrated to the vacated lower IDs.

#### Nodes

The node representation contains 3 top-level elements:

1. The node's internal ID.
2. Any labels associated with the node.
3. The key-value pairs of all properties the node possesses.

```sh
[	
    [“id”, ID (integer)]
    [“labels”,
        [label (string) X label count]
    ]
    [properties”, [
        [prop_key (string), prop_val (scalar)] X property count]
    ]
]
```

#### Relations

The relation representation contains 5 top-level elements:

1. The relation's internal ID.
2. The type associated with the relation.
3. The source node's internal ID.
4. The destination node's internal ID.
5. The key-value pairs of all properties the relation possesses.

```sh
[	
    [“id”, ID (integer)]
    [“type”, type (string)]
    [“src_node”, source node ID (integer)]
    [“dest_node”, destination node ID (integer)]
    [properties”, [
        [prop_key (string), prop_val (scalar)] X property count]
    ]
]
```

### Collections

#### Arrays
When array values are specified in a RETURN clause, the representation in the response is the array string representation. This is done solely for better readability of the response.
The string representation of an array which contains graph entities, will print only their ID. A node string representation is round braces around its ID, and edge string representation is brackets around the edge ID.

#### Paths
Returned path value is the string representation of an array with the path's nodes and edges, interleaved.

## Example

Given the graph created by the command:

```sh
CREATE (:person {name:'Pam', age:27})-[:works {since: 2010}]->(:employer {name:'Dunder Mifflin'})""
```

We can run a query that returns a node, a relation, and a scalar value.

```sh
"MATCH (n1)-[r]->(n2) RETURN n1, r, n2.name"
1) 1) "n1"
   2) "r"
   3) "n2.name"
2) 1) 1) 1) 1) "id"
            2) (integer) 0
         2) 1) "labels"
            2) 1) "person"
         3) 1) "properties"
            2) 1) 1) "age"
                  2) (integer) 27
               2) 1) "name"
                  2) "Pam"
      2) 1) 1) "id"
            2) (integer) 0
         2) 1) "type"
            2) "works"
         3) 1) "src_node"
            2) (integer) 0
         4) 1) "dest_node"
            2) (integer) 1
         5) 1) "properties"
            2) 1) 1) "since"
                  2) (integer) 2010
      3) "Dunder Mifflin"
3) 1) "Query internal execution time: 1.858986 milliseconds"
```


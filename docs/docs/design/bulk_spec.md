---
title: "Implementation details for the GRAPH.BULK endpoint"
linkTitle: "GRAPH.BULK endpoint"
weight: 12
description: >
    The RedisGraph bulk loader uses the GRAPH.BULK endpoint to build a new graph from 1 or more Redis queries.
    The bulk of these queries is binary data that is unpacked to create nodes, edges, and their properties.
    This endpoint could be used to write bespoke import tools for other data formats using the implementation details provided here.
---

## Caveats
The main complicating factor in writing bulk importers is that Redis has a maximum string length of 512 megabytes and a default maximum query size of 1 gigabyte. As such, large imports must be written incrementally.

The RedisGraph team will do their best to ensure that future updates to this logic do not break current implementations, but cannot guarantee it.

## Query Format

```
GRAPH.BULK [graph name] ["BEGIN"] [node count] [edge count] ([binary blob] * N)
```

### Arguments
#### graph name
The name of the graph to be inserted.

#### BEGIN
The endpoint cannot be used to update existing graphs, only to create new ones. For this reason, the first query in a sequence of BULK commands should pass the string literal "BEGIN".

#### node count
Number of nodes being inserted in this query.

#### edge count
Number of edges being inserted in this query.

#### binary blob
A binary string of up to 512 megabytes that partially or completely describes a single label or relationship type.

Any number of these blobs may be provided in a query provided that Redis's 1-gigabyte query limit is not exceeded.

### Module behavior
The endpoint will parse binary blobs as nodes until the number of created nodes matches the node count, then will parse subsequent blobs as edges. The import tool is expected to correctly provide these counts.

If the `BEGIN` token is found, the module will verify that the graph key is unused, and will emit an error if it is. Otherwise, the partially-constructed graph will be retrieved in order to resume building.

## Binary Blob format

### Node format
Nodes in node blobs do not need to specify their IDs. The ID of each node is an 8-byte unsigned integer corresponding to the node count at the time of its creation. (The first-created node has the ID of 0, the second has 1, and so forth.)

The blob consists of:

1. [header specification](#header-specification)

2. 1 or more [property specifications](#property-specification)

### Edge format
The import tool is responsible for tracking the IDs of nodes used as edge endpoints.

The blob consists of:

1. [header specification](#header-specification)

2. 1 or more:
    1. 8-byte unsigned integer representing source node ID
    2. 8-byte unsigned integer representing destination node ID
	3. [property specification](#property-specification)


#### Header specification
1. `name` - A null-terminated string representing the name of the label or relationship type.

2. `property count` - A 4-byte unsigned integer representing the number of properties each entry in this blob possesses.

3. `property names` - an ordered sequence of `property count` null-terminated strings, each representing the name for the property at that position.

#### Property specification
1. `property type` - A 1-byte integer corresponding to the [TYPE enum](https://github.com/RedisGraph/RedisGraph/blob/master/src/bulk_insert/bulk_insert.c#L14-L23):
```sh
BI_NULL = 0,
BI_BOOL = 1,
BI_DOUBLE = 2,
BI_STRING = 3,
BI_LONG = 4,
BI_ARRAY = 5,
```

2. `property`:
    * 1-byte true/false if type is boolean
    * 8-byte double if type is double
    * 8-byte integer if type is integer
    * Null-terminated C string if type is string
    * 8-byte array length followed by N values of this same type-property pair if type is array


## Redis Reply
Redis will reply with a string of the format:
```
[N] nodes created, [M] edges created
```

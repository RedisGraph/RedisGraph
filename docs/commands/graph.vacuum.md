# Vacuum

Graphs when used overtime might accumulate internal fragmentation, this can lead to excessive memory consumption in addition to performance degradation.
To mitigate fragmentation we've introduced the GRAPH.VACCUM command which performs defragmentation.

# Fragments

How does a graph gets fragmented?

To answer this question we first need to understand how does RedisGraph stores its nodes and edges in memory.
At the storage level nodes and edges are considered the same, both are `Graph Entities` composed of a unique ID and optionally an `attribute-set`.

For example consider the `Person` node with internal ID `12` and the attribute-set: `{'name': 'Kenny', 'birth-year': 1984}`
and the Edge of type `Follows` with internal ID `3` and the attribute-set: `{'since':2021}`.

In order to store graph entities in memory RedisGraph maintains two separated containers,
one for holding nodes and another for storing edges,
these containers also referred to as `DataBlocks` in the RedisGraph source code,
can be thought of as arrays.
To locate a GraphEntity with ID `K` one can access the relevant DataBlock at index `K`.

Each item within the `DataBlock` is in one of these three states:
1. Active (containing an entity)
2. Empty (ready to accommodate a future entity)
3. Mark as deleted

Whenever an entity is deleted the corresponding `datablock` entry is marked as deleted
and the entry's index (the entity's ID) is add to the datablock `free-list` which keeps track of all of the deleted entries.
When a new graph entity is created RedisGraph consults with the `free-list` to see if an entry can be reused,
in case the `free-list` is empty then the new entity is added to the end of the `datablock` otherwise a deleted entry is reused
and the newly created node/edge is assigned an "old" ID.

Datablock with 3 active entries, no deleted entries and 4 empty slots.
|0|1|2|3|4|5|6|
|--|--|--|--|--|--|--|--|
|A|A|A|_|_|_|_|_|

Creating a new entity
Tacks to the first empty slot, entity is assign ID 3.
|0|1|2|*3*|4|5|6|
|--|--|--|--|--|--|--|--|
|A|A|A|**A**|_|_|_|_|

Deleting entry with ID 2.
|0|1|*2*|3|4|5|6|
|--|--|--|--|--|--|--|--|
|A|A|**D**|A|_|_|_|_|

Creating a new entity
Reuse deleted slot, entity is assigned ID 2.
|0|1|*2*|3|4|5|6|
|--|--|--|--|--|--|--|--|
|A|A|**A**|A|_|_|_|_|

It is this concept of entry reuse that can lead to graph fragmentation.
Consider a use case where a large number of nodes are created and shortly after get discarded.
Unless a new set of nodes is created the graph will maintain a highly fragmented datablock.
To perform defragmentation and compact datablocks one can issue the `GRAPH.VACUUM` command.

# Defragmentation
The process of `datablock` defrag takes the following steps:
1. Sort `freelist`
2. Migrate active entries from the end of the `datablock` to deleted entries at the beginning of the `datablock`
3. Find the last active entry, delete all entries following it and clear the `freelist`

Once the `freelist` is sorted active entries from the end of the datablock are migrated to the lowest deleted entry available.
This migration process ends when there are no more active entries at position greater than a deleted entry.
This leaves us with a `datablock` of the following structure:
|0|1|2|3|4|5|6|
|--|--|--|--|--|--|--|--|
|A|A|A|D|D|_|_|_|

Lastly the defragmentation process trims all entries following the last active entry and clears the `datablock's` `freelist` Leaving us withe the following `datablock`
|0|1|2|
|--|--|--|
|A|A|A|

# GRAPH.VACUUM key

The command `GRAPH.VACUUM` requires a graph key to operate on e.g. `GRAPH.VACUUM G` will defrag the graph `G`
Please note: this command is considered an administrative command which will block access to the graph throughout its execution. moreover if indeed graph entities had been migrated within the `datablock` their internal ID is changed.

# Example

Create a graph with 5 million nodes
```cypher
127.0.0.1:6379> GRAPH.QUERY g "UNWIND range(0, 5000000) as x CREATE (:Person)"
1) 1) "Labels added: 1"
2) "Nodes created: 5000001"
3) "Cached execution: 0"
4) "Query internal execution time: 909.602583 milliseconds"
```

Delete every node with an odd ID
```cypher
127.0.0.1:6379> GRAPH.QUERY g "MATCH(n:Person) WHERE ID(n) % 2 = 1 DELETE n"
1) 1) "Nodes deleted: 2500000"
2) "Cached execution: 0"
3) "Query internal execution time: 1848.057667 milliseconds"
(1.85s)
```

Ask Redis for its memory consumption
```sh
127.0.0.1:6379> info memory
# Memory
used_memory:497975792
used_memory_human:474.91M
```

Vacuum
```sh
127.0.0.1:6379> GRAPH.VACUUM g
OK
(0.63s)
```

Ask Redis for its memory consumption
```sh
127.0.0.1:6379> info memory
# Memory
used_memory:226676096
used_memory_human:216.18M
```

As can be seen memory consumption has been reduced by almost half.

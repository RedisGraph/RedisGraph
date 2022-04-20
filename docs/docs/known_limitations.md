---
title: "Known limitations"
linkTitle: "Known limitations"
weight: 9
description: ""
---

## Relationship uniqueness in patterns

When a relation in a match pattern is not referenced elsewhere in the query, RedisGraph will only verify that at least one matching relation exists (rather than operating on every matching relation).

In some queries, this will cause unexpected behaviors. Consider a graph with 2 nodes and 2 relations between them:

```
CREATE (a)-[:e {val: '1'}]->(b), (a)-[:e {val: '2'}]->(b)
```

Counting the number of explicit edges returns 2, as expected.

```
MATCH (a)-[e]->(b) RETURN COUNT(e)
```

However, if we count the nodes in this pattern without explicitly referencing the relation, we receive a value of 1.

```
MATCH (a)-[e]->(b) RETURN COUNT(b)
```

We are researching designs that resolve this problem without negatively impacting performance. As a temporary workaround, queries that must operate on every relation matching a pattern should explicitly refer to that relation's alias elsewhere in the query. Two options for this are:

```
MATCH (a)-[e]->(b) WHERE ID(e) >= 0 RETURN COUNT(b)
MATCH (a)-[e]->(b) RETURN COUNT(b), e.dummyval
```

## LIMIT clause does not affect eager operations

When a WITH or RETURN clause introduces a LIMIT value, this value ought to be respected by all preceding operations.

For example, given the query:

```
UNWIND [1,2,3] AS value CREATE (a {property: value}) RETURN a LIMIT 1
```

One node should be created with its 'property' set to 1. RedisGraph will currently create three nodes, and only return the first.

This limitation affects all eager operations: CREATE, SET, DELETE, MERGE, and projections with aggregate functions.

## Indexing limitations

One way in which RedisGraph will optimize queries is by introducing index scans when a filter is specified on an indexed label-property pair.

The current index implementation, however, does not handle not-equal (`<>`) filters.

To profile a query and see whether index optimizations have been introduced, use the `GRAPH.EXPLAIN` endpoint:

```sh
$ redis-cli GRAPH.EXPLAIN social "MATCH (p:person) WHERE p.id < 5 RETURN p"
1) "Results"
2) "    Project"
3) "        Index Scan | (p:person)"
```

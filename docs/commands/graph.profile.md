Executes a query and produces an execution plan augmented with metrics for each operation's execution.

Arguments: `Graph name, Query`

Returns: `String representation of a query execution plan, with details on results produced by and time spent in each operation.`

`GRAPH.PROFILE` is a parallel entrypoint to `GRAPH.QUERY`. It accepts and executes the same queries, but it will not emit results,
instead returning the operation tree structure alongside the number of records produced and total runtime of each operation.

It is important to note that this blends elements of [GRAPH.QUERY](/commands/graph.query) and [GRAPH.EXPLAIN](/commands/graph.explain).
It is not a dry run and will perform all graph modifications expected of the query, but will not output results produced by a `RETURN` clause or query statistics.

```sh
GRAPH.PROFILE imdb
"MATCH (actor_a:Actor)-[:ACT]->(:Movie)<-[:ACT]-(actor_b:Actor)
WHERE actor_a <> actor_b
CREATE (actor_a)-[:COSTARRED_WITH]->(actor_b)"
1) "Create | Records produced: 11208, Execution time: 168.208661 ms"
2) "    Filter | Records produced: 11208, Execution time: 1.250565 ms"
3) "        Conditional Traverse | Records produced: 12506, Execution time: 7.705860 ms"
4) "            Node By Label Scan | (actor_a:Actor) | Records produced: 1317, Execution time: 0.104346 ms"
```


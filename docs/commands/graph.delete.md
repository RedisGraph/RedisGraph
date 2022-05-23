Completely removes the graph and all of its entities.

Arguments: `Graph name`

Returns: `String indicating if operation succeeded or failed.`

```sh
GRAPH.DELETE us_government
```

Note: To delete a node from the graph (not the entire graph), execute a `MATCH` query and pass the alias to the `DELETE` clause:

```
GRAPH.QUERY DEMO_GRAPH "MATCH (x:Y {propname: propvalue}) DELETE x"
```

WARNING: When you delete a node, all of the node's incoming/outgoing relationships are also removed.


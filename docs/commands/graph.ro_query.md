Executes a given read only query against a specified graph.

Arguments: `Graph name, Query, Timeout [optional]`

Returns: [Result set](/redisgraph/design/result_structure) for a read only query or an error if a write query was given.

```sh
GRAPH.RO_QUERY us_government "MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
```

Query-level timeouts can be set as described in [the configuration section](/redisgraph/configuration#timeout).

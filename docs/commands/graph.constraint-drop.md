---
syntax: |
  GRAPH.CONSTRAINT DROP key 
    MANDATORY|UNIQUE
    NODE|RELATIONSHIP
    label|reltype
    PROPERTIES <prop-count> prop [prop...]  
---

Deletes a graph constraint.

See [GRAPH.CONSTRAINT CREATE](https://github.com/RedisGraph/RedisGraph/blob/master/docs/commands/graph.constraint-create.md) for introduction to graph constraints.

To delete a constraint, use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAINT DROP key MANDATORY|UNIQUE NODE|RELATIONSHIP label|reltype PROPERTIES prop-count prop [prop...]
```

For example: to delete the `UNIQUE` constraint created in the example above, issue the following command:

```
GRAPH.CONSTRAINT DROP g UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

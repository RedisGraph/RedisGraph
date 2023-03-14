---
syntax: |
  GRAPH.CONSTRAINT DROP key 
    MANDATORY|UNIQUE
    NODE|RELATIONSHIP
    label/reltype
    PROPERTIES <prop-count> prop [prop...]  
---

Deleted a graph constraint.

[Examples](#examples)

For an introduction to constraints see [GRAPH.CONSTRAINT CREATE](https://github.com/RedisGraph/RedisGraph/blob/master/docs/commands/graph.constraint-create.md)

## Required arguments

<details open><summary><code>key</code></summary>

is key name for the graph.
</details>

<details open><summary><code>constraintType</code></summary>

is the constraint type: either `MANDATORY` or `UNIQUE`.

</details>

<details open><summary><code>entitiesType</code></summary>

is the entities type on which the constraint should be enforced: either `NODE` or `RELATIONSHIP`.

</details>

<details open><summary><code>label/reltype</code></summary>

is the name of the node label or relationship type on which the constraint should be enforced.

</details>

<details open><summary><code>label/propCount</code></summary>

is the number of properties following. Valid values are between 1 and 255.

</details>

<details open><summary><code>label/prop...</code></summary>

is a list of `propCount` property names.

</details>




## Examples

To delete a unique constraint for all nodes with label `Person` enforcing uniqueness on the combination of values of attributes `first_name` and `last_name`, issue the following commands:

```
GRAPH.CONSTRAINT g DROP UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

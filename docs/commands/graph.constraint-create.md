---
syntax: |
  GRAPH.CONSTRAINT CREATE key 
    MANDATORY|UNIQUE
    NODE|RELATIONSHIP
    label/reltype
    PROPERTIES <prop-count> prop [prop...]  
---

Creates a graph constraint.

[Examples](#examples)

## Introduction to constraints

A constraint is a rule enforced on graph entities or relationships, used to guarantee a certain structure of the data.

RedisGraph supports two types of constraints:

1. Mandatory constraints
2. Unique constraints

### Mandatory constraints

A mandatory constraint enforces existence of given attributes for all nodes with a given label or for all edges with a given relationship-type.

Consider a mandatory constraint over the attribute `id` of all nodes with the label `Person`.
This constraint will enforce that any `Person` node in the graph has an `id` attribute.
Any attempt to create or modify a `Person` node, such that the resulting node does not have an `id` attribute, will fail.

### Unique constraints

A unique constraint enforces uniqueness of values of a given set of attributes for all nodes with a given label or for all edges with a given relationship-type. I.e., no duplicates are allowed.

Consider a unique constraint over the attributes: `first_name` and `last_name` of all nodes with the label `Person`
This constraint will enforce that any combination of `first_name`, `last_name` is unique.
E.g., a graph can contain the following `Person` nodes:

```
(:Person {first_name:'Frank', last_name:'Costanza'})
(:Person {first_name:'Estelle', last_name:'Costanza'})
```

But trying to create a third node with `first_name` Frank and `last_name` Costanza, will issue an error and the query will fail.

<note><b>Notes:</b>

- A unique constraint requires the existence of an exact-match index prior to its creation. For example, trying to create a unique constraint governing attributes: `first_name` and `last_name` of nodes with label `Person` without having an exact-match index over `Person`'s `first_name` and `last_name` attributes will fail.
   
- A unique constraint is enforced for a given node/edge only if all the constrainted properties are set (non-null).
- Unique constraints are not enforced for array-valued properties.
- Trying to delete an index that supports a constraint will fail.
   
</note>

## Creating a constraint

To create a constraint, use the `GRAPH.CONSTRAINT CREATE` command as folllows:

```
GRAPH.CONSTRAINT CREATE key constraintType entitiesType label/reltype PROPERTIES propCount prop [prop...]
```

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

<note><b>Notes:</b>

- Constraints are created asynchronously. The constraint creation command will reply with `PENDING` and the newly created constraint is enforced gradually on all relevant entities.
  During its creation phase, a constraint's status is `PENDING`. If all governed entities confirm to the constraint - its status is updated to `OPERATIONAL`, otherwise, if a conflicting entity has been detected, the constraint status is updated to `FAILED` and the constraint is not enforced. The caller may try to resolve the conflict and recreate the constraint. To retrieve the status of all constraints - use the `db.constraints()` procedure.
- A constraint creation command may fail synchronously due to the following reasons:
  1. Syntax error
  2. Constraint already exists
  3. Missing supporting index (for unique constraint)

  In addition, a constraint creation command may fail asynchronously due to the following reasons:

  1. The graph contains data which violates the constraint

</note>

## Examples

To create a unique constraint for all nodes with label `Person` enforcing uniqueness on the combination of values of attributes `first_name` and `last_name`, issue the following commands:

```
GRAPH.QUERY g "CREATE INDEX FOR (p:Person) ON (p.first_name, p.last_name)"
GRAPH.CONSTRAINT CREATE g UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

Similarly, to create a mandatory constraint for all edges with relationship-type `Visited`, enforcing the existence of a `date` attribute, issue the following command:

```
GRAPH.CONSTRAINT CREATE g MANDATORY RELATIONSHIP Visited PROPERTIES 1 date
```

## Deleting a constraint

See [GRAPH.CONSTRAINT DROP](https://github.com/RedisGraph/RedisGraph/blob/master/docs/commands/graph.constraint-drop.md)

## Listing constraints

To list all constraints on a given graph, use the `db.constraints` procedure:

```
GRAPH.RO_QUERY <key> "CALL db.constraints()"
```

For each constraint the procedure will yield the following fields:

| Field        | Desc                                                   |
| ------------ | ------------------------------------------------------ |
| `type`       | type of constraint, either `UNIQUE` or `MANDATORY`     |
| `label`      | label or relationship-type enforced by constraint      |
| `properties` | list of properties enforced by constraint              |
| `entitytype` | type of entity, either `NODE` or `RELATIONSHIP`        |
| `status`     | either `UNDER CONSTRUCTION`, `OPERATIONAL` or `FAILED` |

Example:

```
127.0.0.1:6379> GRAPH.RO_QUERY g "call db.constraints()"
1) 1) "type"
   2) "label"
   3) "properties"
   4) "entitytype"
   5) "status"
2) 1) 1) "UNIQUE"
      2) "Person"
      3) "[birthdate]"
      4) "NODE"
      5) "UNDER CONSTRUCTION"
   2) 1) "MANDATORY"
      2) "Person"
      3) "[first_name, last_name]"
      4) "NODE"
      5) "OPERATIONAL"
```

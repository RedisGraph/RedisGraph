Creates or deletes a graph constraint. 

A constraint is a rule enforced on graph entities or relationships, used to guarantee a certian structure of the data.

RedisGraph supports two types of constraints:

1. Mandatory constraints
2. Unique constraints

## Mandatory constraints

A mandatory constraint enforces existance of given attributes for all nodes with a given label or for all edges with a given relationship-type.

Consider a mandatory constraint over the attribute `id` of all nodes with the label `Person`.
This constraint will enforce that any `Person` node in the graph has an `id` attribute.
Any attempt to create or modify a `Person` node, such that the resulting node does not have an `id` attribute, will fail.

## Unique constraints

A unique constraint enforces uniquness of values of a given set of attributes for all nodes with a given label or for all edges with a given relationship-type. I.e., no duplicates are allowed.

Consider a unique constraint over the attributes: `first_name` and `last_name` of all nodes with the label `Person`
This constraint will enforce that any combination of `first_name`, `last_name` is unique.
E.g., a graph can contain the following `Person` nodes:

```
(:Person {first_name:'Frank', last_name:'Costanza'})
(:Person {first_name:'Estelle', last_name:'Costanza'})
```

But trying to create a third node with `first_name` Frank and `last_name` Costanza, will issue an error and the query will fail.

<note><b>Notes:</b>

- A unique constraint requires the existance of an exact-match index prior to its creation. For example, trying to create a unique constraint governing attributes: `first_name` and `last_name` of entities with label `Person` without having an exact-match index over `Person`'s `first_name` and `last_name` attributes will fail.
   
- Trying to delete an index that supports a constraint will fail.
   
- Unique constraints are enforced only if all the constrainted properties are present (non-null).
   
- Unique constraints are not enforced for array-valued properties.
   
</note>

## Creating constraints

To create a constraint, use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAINT <key> CREATE MANDATORY|UNIQUE NODE|RELATIONSHIP <label/reltype> PROPERTIES <prop-count> prop [prop...]
```

For example, to create a unique constraint for all nodes with label `Person`, enforcing uniquness on the combination of values of attributes `first_name` and `last_name`, issue the following commands:

```
GRAPH.QUERY g "CREATE INDEX FOR (p:Person) ON (p.first_name, p.last_name)"
GRAPH.CONSTRAINT g CREATE UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

Similarly to create a mandatory constraint for all edges with relationship-type `Visited`, enforcing the existence of a `date` attribute, issue the following command:

```
GRAPH.CONSTRAINT g CREATE MANDATORY RELATIONSHIP Visited PROPERTIES 1 date
```

<note><b>Note:</b>

Constraints are created asynchronously. The constraint creation command will reply `OK`, and the newly created constraint is enforced gradually on all relevant entities.

During its creation phase, a constraint is considered `PENDING`. If all governed entities confirm with the constraint - its status is updated to `OPERATIONAL`, otherwise, if a conflicting entity has been detected, the constraint status is updated to `FAILURE` and the constraint is not enforced. The caller can resolve the conflict and recreate the constraint. To query constraints status use the `db.constraints()` procedure.
   
</note>

A constraint creation command may fail synchronously due to the following reasons:

1. Syntax error
2. Constraint already exists
3. Missing supporting index (for unique constraint)

In addition, a constraint creation command may fail asynchronously due to the following reasons:

1. The graph already contains data which violates the constraint

## Listing constraints

To list all constraints in a given graph, use the `db.constraints` procedure:

```
GRAPH.QUERY <key> "CALL db.constraints()"
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
127.0.0.1:6379> GRAPH.QUERY g "call db.constraints()"
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

## Deleting constraints

To delete a constraint, use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAINT <key> DROP MANDATORY|UNIQUE NODE|RELATIONSHIP <label/reltype> PROPERTIES <prop-count> prop [prop...]
```

For example: to delete the `UNIQUE` constraint created in the example above, issue the following command:

```
GRAPH.CONSTRAINT g DROP UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

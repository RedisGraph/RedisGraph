# Constraints

A constraint is a rule enforced on graph entities, it's used to guarantee a certian structure of the data.

There are two types of constraints:

1. MANDATORY
2. UNIQUE

## Mandatory constraint

An mandatory constraint enforces the existance of attribute(s).

Consider a mandatory constraint over the attribute `id` of all nodes with the label `Person`.
This constraint will enforce that any `Person` node in the graph has an `id` attribute.
Any attempt to create or modify a `Person` node, such that the resulting node does not have an `id` attribute, will fail.

## Unique constraint

A unique constraint enforces uniquness over attribute value(s), i.e. no duplicates are allowed.

Consider a unique constraint over the attributes: `first_name` and `last_name` of all nodes with the label `Person`
This constraint will enforce that any combination of `first_name`, `last_name`  is unique.
E.g., a graph can contain the following `Person` nodes:

```
(:Person {first_name:'Frank', last_name:'Costanza'})
(:Person {first_name:'Estelle', last_name:'Costanza'})
```

But trying to create a third node with `first_name` Frank and `last_name` Costanza, will issue an error and the query will fail.

<note><b>Notes:</b>

- A unique constraint requires the existance of an exact-match index prior to its creation. For example, trying to create a unique constraint governing attributes: `first_name` and `last_name` of entities with label `Person` without having an exact-match index over `Person`'s `first_name` and `last_name` attributes will fail.
   
- Trying to delete an index that supports a constraint will fail.
   
- Unique constraints are not enforced for array-valued properties.
   
</note>

## Create constraints

To create a constraint use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAINT <key> CREATE MANDATORY|UNIQUE NODE|RELATIONSHIP <label/reltype> PROPERTIES <prop-count> prop [prop...]
```

For example, to create a unique constraint for all nodes with label `Person`, enforcing uniquness on the combination of values of attributes `first_name` and `last_name`, issue the following commands:

```
GRAPH.QUERY g "CREATE INDEX FOR (p:Person) ON (p.first_name, p.last_name)"
GRAPH.CONSTRAINT g CREATE UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

Similarly, to create a mandatory constraint for all edges with relationship-type `Visited`, enforcing the existence of a `date` attribute, issue the following command:

```
GRAPH.CONSTRAINT g CREATE MANDATORY RELATIONSHIP Visited PROPERTIES 1 date
```

<note><b>Note:</b>

Constraints are created asynchronously. The constraint creation command will reply `OK`, and the newly created constraint is enforced gradually on all relevant entities.

During its creation phase, a constraint is considered `PENDING`. If all governed entities confirm with the constraint - its status is updated to `OPERATIONAL`, otherwise, if a conflicting entity has been detected, the constraint status is updated to `FAILURE` and the constraint is not enforced. The caller can resolve the conflict and recreate the constraint. To query constraints status use the `db.constraints()` procedure.
   
</note>

The constraint creation command can fail for the following reasons:

1. syntax error
2. constraint already exists
3. missing supporting index

## Listing constraints

To list all constraints in a graph use the `db.constraints` procedure

```
GRAPH.QUERY <key> "CALL db.constraints()"
```

For each constraint the procedure will yield the following fields:

| Field        | Desc                                                   |
| ------------ | ------------------------------------------------------ |
| `type`       | type of constraint, either `unique` or `mandatory`     |
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
2) 1) 1) "unique"
      2) "Person"
      3) "[birthdate]"
      4) "NODE"
      5) "UNDER CONSTRUCTION"
   2) 1) "mandatory"
      2) "Person"
      3) "[first_name, last_name]"
      4) "NODE"
      5) "OPERATIONAL"
```

## Deleting constraints

To delete a constraint use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAINT <key> DROP MANDATORY|UNIQUE NODE|RELATIONSHIP <label/reltype> PROPERTIES <prop-count> prop [prop...]
```

For example: to delete the unique constraint created in the example above, issue the following command:

```
GRAPH.CONSTRAINT g DROP UNIQUE NODE Person PROPERTIES 2 first_name last_name
```

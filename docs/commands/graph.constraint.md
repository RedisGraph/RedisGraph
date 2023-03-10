# Constraints

A constraint is a rule enforced on graph entities, it's used to guarantee a certian structure of the data.

There are two types of constraints:

1. MANDATORY
2. UNIQUE

## Mandatory constraint

An mandatory constraint enforces the existance of attribute(s).

Consider a mandatory constraint over the attribute `id` of all nodes with the label `Person`.
The constraint will make sure that all `Person` nodes in the graph will have an `id` attribute.
Any attempt to modify or create a `Person` node which doesn't have an `id` attribute will fail.

## Unique constraint

A unique constraint enforces uniquness over attribute value(s), i.e. no duplicates are allowed.

Consider a unique constraint over the attributes: `first_name` and `last_name` of all nodes with the label `Person`
The constraint will make sure that any combination of `first_name`, `last_name`  is unique.
e.g. a graph can contain the following `Person` nodes:

```
(:Person {first_name:'Frank', last_name:'Costanza'})
(:Person {first_name:'Estelle', last_name:'Costanza'})
```

But if we'll try to create a third node with `first_name` Frank and `last_name` Costanza, we'll get an error and the node will not be created.

Please note: a unique constraint requires the existance of an `exact-match` index prior to its creation.
Trying to create a unique constraint governing attributes: `first_name` and `last_name` of entities of type `Person` without having an `exact-match` index over `Person's`  `first_name` and `last_name` attributes will lead to a constraint creation failure.

## Create constraint

To create a constraint use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAIN <key> CREATE <constraint-type> <entity-type> <label/reltype> PROPERTIES <prop-count> prop0, prop1...
```

For example to create a `unique` constraint against all nodes with label `Person` enforcing uniquness on the combined attributes: `first_name` and `last_name` issue the following command:

```
GRAPH.CONSTRAIN g CREATE UNIQUE NODE Person PROPERTIES 2 first_name, last_name
```

Similarly to create a `mandatory` constraint against all edges with relationship type `Visited` enforcing the existence of the `date` attribute issue the following command:

```
GRAPH.CONSTRAIN g CREATE MANDATORY RELATIONSHIP Visited PROPERTIES 1 date
```

Please note: constraints are created asynchronously, the constraint creation command will reply `OK` instantly to the caller and the newly created constraint is enforced gradually on all relevant entities.

During its creation phase a constraint is considered `pending`, if all govenared entities confirms with the constraint its status is updated to `operational`, otherwise a conflicting entity has been detected, the constraint status is updated to `failure`, in such case the constraint isn't enforced, the caller can resolve the conflict and recreate the constraint. To query constraints status use the `db.constraints()` procedure.

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

| Field        | Desc                                                  |
| ------------ | ----------------------------------------------------- |
| `type`       | type of constraint, either `Unique` or `Mandatory`    |
| `label`      | label or relationship-type enforced by constraint     |
| `properties` | list of properties enforced by constraint             |
| `entitytype` | type of entity, either `Node` or `Relation`           |
| `status`     | either `under construction`, `operational or `failed` |

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

## Deleting constraint

To delete a constraint use the `GRAPH.CONSTRAINT` command as folllows:

```
GRAPH.CONSTRAIN <key> DROP <constraint-type> <entity-type> <label/reltype> PROPERTIES <prop-count> prop0, prop1...
```

For example: to delete the unique constraint created in the example above, issue the following command:

```
GRAPH.CONSTRAIN g DROP UNIQUE LABEL Person PROPERTIES 2 first_name, last_name
```

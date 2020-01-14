/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./ops/op.h"
#include "../graph/graph.h"
#include "../resultset/resultset.h"
#include "../filter_tree/filter_tree.h"

typedef struct ExecutionPlan ExecutionPlan;

struct ExecutionPlan {
	OpBase *root;                       // Root operation of overall ExecutionPlan.
	rax *record_map;                    // Mapping between identifiers and record indices.
	ResultSet *result_set;              // ResultSet populated by this query.
	QueryGraph *query_graph;            // QueryGraph representing all graph entities in this segment.
	FT_FilterNode *filter_tree;         // FilterTree containing filters to be applied to this segment.
	QueryGraph **connected_components;  // Array of all connected components in this segment.
	// NOTE - segments and segment_count are only stored for proper freeing.
	int segment_count;                  // Number of ExecutionPlan segments.
	ExecutionPlan **segments;           // Partial execution plans scoped to a subset of operations.
	// Semi-independent sub execution plans which created during the build of the main execution plan.
	ExecutionPlan **sub_execution_plans;
    DataBlock *record_pool;
};

/* execution_plan_modify.c
 * Helper functions to move and analyze operations in an ExecutionPlan. */

/* Removes operation from execution plan. */
void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op);

/* Detaches operation from its parent. */
void ExecutionPlan_DetachOp(OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp);

/* Push b right below a. */
void ExecutionPlan_PushBelow(OpBase *a, OpBase *b);

/* Introduce new_root as the parent of old_root. */
void ExecutionPlan_NewRoot(OpBase *old_root, OpBase *new_root);

/* Replace a with b. */
void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b);

/* Traverse upwards until an operation that resolves the given alias is found.
 * Returns NULL if alias is not resolved. */
OpBase *ExecutionPlan_LocateOpResolvingAlias(OpBase *root, const char *alias);

/* Locate the first operation of a given type within execution plan.
 * Returns NULL if operation wasn't found. */
OpBase *ExecutionPlan_LocateFirstOp(OpBase *root, OPType type);

/* Locate the last operation of a given type within execution plan.
 * Returns NULL if operation wasn't found. */
OpBase *ExecutionPlan_LocateLastOp(OpBase *root, OPType type);

/* Locate all operations of a given type within execution plan.
 * Returns an array of operations. */
OpBase **ExecutionPlan_LocateOps(OpBase *root, OPType type);

/* Find the earliest operation above the provided recurse_limit, if any,
 * at which all references are resolved. */
OpBase *ExecutionPlan_LocateReferences(OpBase *root, const OpBase *recurse_limit,
									   rax *references_to_resolve);

/* Populate a rax with all aliases that have been resolved by the given operation
 * and its children. These are the bound variables at this point in execution, and
 * subsequent operations should not introduce them as new entities. For example, in the query:
 * MATCH (a:A) CREATE (a)-[:E]->(b:B)
 * The Create operation should never introduce a new node 'a'. */
void ExecutionPlan_BoundVariables(const OpBase *op, rax *modifiers);

/* Build an array of const strings to populate the 'modifies' arrays of Argument ops.
 * Use after the call to ExecutionPlan_BoundVariables. */
const char **ExecutionPlan_BuildArgumentModifiesArray(rax *bound_vars);

/* For all ops in the given tree, assocate the provided ExecutionPlan.
 * This is for use for updating ops that have been built with a temporary ExecutionPlan. */
void ExecutionPlan_BindPlanToOps(ExecutionPlan *plan, OpBase *root);

/* Adds a semi-independent sub execution plan. The only thing the sub execution plan is dependent on is the record mapping. */
void ExecutionPlan_AppendSubExecutionPlan(ExecutionPlan *master_plan, ExecutionPlan *sub_plan);

/* execution_plan.c */

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(ResultSet *result_set);

/* Allocate a new ExecutionPlan segment. */
ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void);

/* Build a tree of operations that performs all the work required by the clauses of the current AST. */
void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan, ResultSet *result_set);

// TODO: Remove this once filter are placed after their respective clause.
/* Place filter ops*/
void ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan, const OpBase *recurse_limit);

/* Retrieve the map of aliases to Record offsets in this ExecutionPlan segment. */
rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan);

/* Retrieves a record from records pool. */
Record ExecutionPlan_BorrowRecord(ExecutionPlan *plan);

/* Free record - returns record to records pool. */
void ExecutionPlan_ReturnRecord(ExecutionPlan *plan, Record r);

/* Prints execution plan. */
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx);

/* Initialize all operations in an ExecutionPlan. */
void ExecutionPlan_Init(ExecutionPlan *plan);

/* Executes plan */
ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan);

/* Profile executes plan */
ResultSet *ExecutionPlan_Profile(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlan_Free(ExecutionPlan *plan);


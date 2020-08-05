/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./ops/op.h"
#include "../graph/graph.h"
#include "../resultset/resultset.h"
#include "../filter_tree/filter_tree.h"
#include "../util/object_pool/object_pool.h"

typedef struct ExecutionPlan ExecutionPlan;

struct ExecutionPlan {
	OpBase *root;                       // Root operation of overall ExecutionPlan.
	AST *ast_segment;                   // The segment which the current ExecutionPlan segment is built from.
	rax *record_map;                    // Mapping between identifiers and record indices.
	QueryGraph *query_graph;            // QueryGraph representing all graph entities in this segment.
	FT_FilterNode *filter_tree;         // FilterTree containing filters to be applied to this segment.
	QueryGraph **connected_components;  // Array of all connected components in this segment.
	ExecutionPlan **segments;           // Partial execution plans scoped to a subset of operations.
	ObjectPool *record_pool;
	bool prepared;                      // Indicates if the execution plan is ready for execute.
	bool is_union;                      // Indicates if the execution plan is a union of execution plans.
};

/* execution_plan_modify.c
 * Helper functions to move and analyze operations in an ExecutionPlan. */

/*
 * API for restructuring the op tree.
 */
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

/*
 * ExecutionPlan_Locate API:
 * For performing existence checks and looking up individual operations in tree.
 */
/* Traverse upwards until an operation that resolves the given alias is found.
 * Returns NULL if alias is not resolved. */
OpBase *ExecutionPlan_LocateOpResolvingAlias(OpBase *root, const char *alias);

/* Locate the first operation matching one of the given types in the op tree by performing DFS.
 * Returns NULL if no matching operation was found. */
OpBase *ExecutionPlan_LocateOpMatchingType(OpBase *root, const OPType *types, uint type_count);

/* Convenience wrapper around ExecutionPlan_LocateOpMatchingType for lookups of a single type.
 * Locate the first operation of a given type within execution plan by performing DFS.
 * Returns NULL if operation wasn't found. */
OpBase *ExecutionPlan_LocateOp(OpBase *root, OPType type);

/* Find the earliest operation above the provided recurse_limit, if any,
 * at which all references are resolved. */
OpBase *ExecutionPlan_LocateReferences(OpBase *root, const OpBase *recurse_limit,
									   rax *references_to_resolve);

/* ExecutionPlan_Collect API:
 * For collecting all matching operations in tree. */
/* Collect all operations matching the given types in the op tree.
 * Returns an array of operations. */
OpBase **ExecutionPlan_CollectOpsMatchingType(OpBase *root, const OPType *types, uint type_count);

/* Convenience wrapper around ExecutionPlan_LocateOpMatchingType for
 * collecting all operations of a given type within the op tree.
 * Returns an array of operations. */
OpBase **ExecutionPlan_CollectOps(OpBase *root, OPType type);

/*
 * API for building and relocating operations in transient ExecutionPlans.
 */
/* Populate a rax with all aliases that have been resolved by the given operation
 * and its children. These are the bound variables at this point in execution, and
 * subsequent operations should not introduce them as new entities. For example, in the query:
 * MATCH (a:A) CREATE (a)-[:E]->(b:B)
 * The Create operation should never introduce a new node 'a'. */
void ExecutionPlan_BoundVariables(const OpBase *op, rax *modifiers);

/* For all ops in the given tree, assocate the provided ExecutionPlan.
 * This is for use for updating ops that have been built with a temporary ExecutionPlan. */
void ExecutionPlan_BindPlanToOps(ExecutionPlan *plan, OpBase *root);

/* Given an AST path pattern, generate the tree of scan, traverse,
 * and filter operations required to represent it. */
OpBase *ExecutionPlan_BuildOpsFromPath(ExecutionPlan *plan, const char **vars,
									   const cypher_astnode_t *path);

/* execution_plan.c */

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(void);

// Sets an AST segment in the execution plan.
void ExecutionPlan_SetAST(ExecutionPlan *plan, AST *ast);

// Gets the AST segment from the execution plan.
AST *ExecutionPlan_GetAST(const ExecutionPlan *plan);

/* Prepare an execution plan for execution: optimize, initialize result set schema. */
void ExecutionPlan_PreparePlan(ExecutionPlan *plan);

/* Allocate a new ExecutionPlan segment. */
ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void);

/* Build a tree of operations that performs all the work required by the clauses of the current AST. */
void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan);

/* Re position filter op. */
void ExecutionPlan_RePositionFilterOp(ExecutionPlan *plan, OpBase *lower_bound,
									  const OpBase *upper_bound, OpBase *filter);

// TODO: Remove this once filter are placed after their respective clause.
/* Place filter ops*/
void ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan, const OpBase *recurse_limit);

/* Retrieve the map of aliases to Record offsets in this ExecutionPlan segment. */
rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan);

/* Retrieves a Record from the ExecutionPlan's Record pool. */
Record ExecutionPlan_BorrowRecord(ExecutionPlan *plan);

/* Free Record contents and return it to the Record pool. */
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


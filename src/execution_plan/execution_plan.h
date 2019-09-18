/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "record_map.h"
#include "./ops/op.h"
#include "../graph/graph.h"
#include "../resultset/resultset.h"
#include "../filter_tree/filter_tree.h"


/* StreamState
 * Different states in which stream can be at. */
typedef enum {
	StreamUnInitialized,
	StreamConsuming,
	StreamDepleted,
} StreamState;

typedef struct {
	OpBase *root;                      // Root operation of this specific segment.
	RecordMap *record_map;             // Mapping of aliases and entity/AST IDs to Record IDs
	QueryGraph **connected_components; // Array of all connected components in this segment.
	QueryGraph *query_graph;           // QueryGraph representing all graph entities in this segment.
	FT_FilterNode *filter_tree;        // FilterTree containing filters to be applied to this segment.
	AR_ExpNode **projections;          // Expressions to be constructed for a WITH or RETURN clause.
} ExecutionPlanSegment;

typedef struct {
	OpBase *root;                      // Root operation of overall ExecutionPlan.
	ExecutionPlanSegment
	**segments;   // The segments contained in this ExecutionPlan (only stored for proper freeing).
	ResultSet *result_set;             // ResultSet populated by this query.
	uint segment_count;                // Number of segments in query.
} ExecutionPlan;

/* execution_plan_modify.c
 * Helper functions to move and analyze operations in an ExecutionPlan. */

/* Removes operation from execution plan. */
void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp);

/* Push b right below a. */
void ExecutionPlan_PushBelow(OpBase *a, OpBase *b);

/* Replace a with b. */
void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b);

/* Locates all operation which generate data.
 * SCAN, UNWIND, PROCEDURE_CALL, CREATE. */
void ExecutionPlan_LocateTaps(OpBase *root, OpBase ***taps);

/* Locate the first operation of a given type within execution plan.
 * Returns NULL if operation wasn't found. */
OpBase *ExecutionPlan_LocateOp(OpBase *root, OPType type);

/* Locate all operations of a given type within execution plan.
 * Returns an array of operations. */
OpBase **ExecutionPlan_LocateOps(OpBase *root, OPType type);

/* Returns an array of taps; operations which generate data
 * e.g. SCAN operations */
void ExecutionPlan_Taps(OpBase *root, OpBase ***taps);

/* Find the earliest operation on the ExecutionPlan at which all
 * references are resolved. */
OpBase *ExecutionPlan_LocateReferences(OpBase *root, rax *references);

/* execution_plan.c */

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(
	RedisModuleCtx *ctx,    // Module-level context
	GraphContext *gc,       // Graph access and schemas
	ResultSet *result_set
);

/* Prints execution plan. */
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx);

/* Executes plan */
ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan);

/* Profile executes plan */
ResultSet *ExecutionPlan_Profile(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlan_Free(ExecutionPlan *plan);


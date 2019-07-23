/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __EXECUTION_PLAN_H__
#define __EXECUTION_PLAN_H__

#include "./ops/op.h"
#include "../parser/ast.h"
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
	OpBase *root;
	ResultSet *result_set;
	FT_FilterNode *filter_tree;
	QueryGraph **connected_components;
} ExecutionPlan;

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(
    RedisModuleCtx *ctx,    // Module-level context
    AST **ast,              // Query parsed AST
    ResultSet *result_set,  // Result set to be populated if the query returns data
    bool explain            // Construct execution plan, do not execute
);

/* Prints execution plan. */
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx);

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

/* Executes plan */
ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan);

/* Profile executes plan */
ResultSet *ExecutionPlan_Profile(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlanFree(ExecutionPlan *plan);

#endif

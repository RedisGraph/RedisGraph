/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __EXECUTION_PLAN_H__
#define __EXECUTION_PLAN_H__

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
    OpBase *root;
    QueryGraph *query_graph;
    FT_FilterNode *filter_tree;
    // Record input_record;
} ExecutionPlanSegment;

typedef struct {
    ExecutionPlanSegment **segments;
    ResultSet *result_set;
    uint segment_count;
} ExecutionPlan;

/* execution_plan_modify.c
 * Helper functions to move and analyze operations in an ExecutionPlan. */

/* Removes operation from execution plan. */
void ExecutionPlanSegment_RemoveOp(ExecutionPlanSegment *plan, OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void ExecutionPlanSegment_AddOp(OpBase *parent, OpBase *newOp);

/* Push b right below a. */
void ExecutionPlanSegment_PushBelow(OpBase *a, OpBase *b);

/* Replace a with b. */
void ExecutionPlanSegment_ReplaceOp(ExecutionPlanSegment *plan, OpBase *a, OpBase *b);

/* Locate the first operation of a given type within execution plan.
 * Returns NULL if operation wasn't found. */
OpBase* ExecutionPlanSegment_LocateOp(OpBase *root, OPType type);

/* Returns an array of taps; operations which generate data
 * e.g. SCAN operations */
void ExecutionPlanSegment_Taps(OpBase *root, OpBase ***taps);

/* Find the earliest operation on the ExecutionPlan at which all
 * references are resolved. */
OpBase* ExecutionPlanSegment_LocateReferences(OpBase *root, uint *references);

/* execution_plan.c */

/* Creates a new execution plan from AST */
ExecutionPlan* NewExecutionPlan (
    RedisModuleCtx *ctx,    // Module-level context
    GraphContext *gc,       // Graph access and schemas
    bool explain            // Construct execution plan, do not execute
);

/* Prints execution plan. */
char* ExecutionPlan_Print(const ExecutionPlan *plan);

/* Executes plan */
Record ExecutionPlanSegment_Execute(ExecutionPlanSegment *plan);

ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlanFree(ExecutionPlan *plan);

#endif
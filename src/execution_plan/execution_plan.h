/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "execution_plan/ops/op.h"
#include "parser/ast.h"
#include "graph/graph.h"
#include "resultset/resultset.h"
#include "filter_tree/filter_tree.h"

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
    ResultSet *result_set;
    FT_FilterNode *filter_tree;
} ExecutionPlan;

/* Creates a new execution plan from AST */
ExecutionPlan* NewExecutionPlan (
    RedisModuleCtx *ctx,    // Module-level context
    GraphContext *gc,       // Graph access and schemas
    AST **ast,              // Query parsed AST
    bool explain            // Construct execution plan, do not execute
);

/* Prints execution plan. */
char* ExecutionPlanPrint(const ExecutionPlan *plan);

/* Removes operation from execution plan. */
void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp);

/* Push b right below a. */
void ExecutionPlan_PushBelow(OpBase *a, OpBase *b);

/* Replace a with b. */
void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b);

/* Locate the first operation of a given type within execution plan.
 * Returns NULL if operation wasn't found. */
OpBase* ExecutionPlan_LocateOp(OpBase *root, OPType type);

/* Executes plan */
ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlanFree(ExecutionPlan *plan);

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
    FT_FilterNode *filter_tree;
    ResultSet *result_set;
} ExecutionPlan;

/* Creates a new execution plan from AST */
ExecutionPlan* NewExecutionPlan (
    RedisModuleCtx *ctx,    // Module-level context
    GraphContext *gc,       // Graph access and schemas
    AST *ast,               // Query parsed AST
    bool explain            // Construct execution plan, do not execute
);

/* Prints execution plan. */
char* ExecutionPlanPrint(const ExecutionPlan *plan);

/* Removes operation from execution plan. */
void ExecutionPlan_RemoveOp(OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp);

/* Replace a with b. */
void ExecutionPlan_ReplaceOp(OpBase *a, OpBase *b);

/* Executes plan */
ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlanFree(ExecutionPlan *plan);

/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./ops/op.h"
#include "../graph/graph.h"
#include "../resultset/resultset.h"
#include "../filter_tree/filter_tree.h"
#include "../util/object_pool/object_pool.h"

typedef struct ExecutionPlan {
	OpBase *root;                       // Root operation of overall ExecutionPlan.
	AST *ast_segment;                   // The segment which the current ExecutionPlan segment is built from.
	rax *record_map;                    // Mapping between identifiers and record indices.
	QueryGraph *query_graph;            // QueryGraph representing all graph entities in this segment.
	QueryGraph **connected_components;  // Array of all connected components in this segment.
} ExecutionPlan;

// Creates a new execution plan from AST
ExecutionPlan *NewExecutionPlan(void);

// Allocate a new ExecutionPlan segment
ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void);

// Build a tree of operations that performs all the work required by the clauses of the current AST
void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan);

// Re position filter op
void ExecutionPlan_RePositionFilterOp(ExecutionPlan *plan, OpBase *lower_bound,
									  const OpBase *upper_bound, OpBase *filter);

// Increase execution plan reference count
// Free execution plan
void ExecutionPlan_Free(ExecutionPlan *plan);

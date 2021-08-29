/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../../graph/graph.h"
#include "../../../../algorithms/algorithms.h"
#include "../../../ops/op_cond_var_len_traverse.h"
#include "../../../../arithmetic/algebraic_expression.h"

// OP Traverse
typedef struct {
	RT_OpBase op;
	const CondVarLenTraverse *op_desc;
	Graph *g;
	Record r;
	RG_Matrix M;                           // Traversed matrix if using the SimpleConsume routine
	int edgesIdx;                          // Edges set by operation
	uint srcNodeIdx;                       // Node set by operation
	uint destNodeIdx;                      // Node set by operation
	uint minHops;                          // Maximum number of hops to perform
	uint maxHops;                          // Maximum number of hops to perform
	int edgeRelationCount;                 // Length of edgeRelationTypes
	int *edgeRelationTypes;                // Relation(s) we're traversing
	AlgebraicExpression *ae;               // ArithmeticExpression describing op's traversal pattern
	union {
		AllPathsCtx *allPathsCtx;          // Context for collecting all paths
		AllNeighborsCtx *allNeighborsCtx;  // Context for collecting all neighbors
	};
	bool collect_paths;                    // Whether we must populate the entire path
} RT_CondVarLenTraverse;

RT_OpBase *RT_NewCondVarLenTraverseOp(const RT_ExecutionPlan *plan, const CondVarLenTraverse *op_desc);

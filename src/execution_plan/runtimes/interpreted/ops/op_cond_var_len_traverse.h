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
#include "../../../../arithmetic/algebraic_expression.h"

// OP Traverse
typedef struct {
	RT_OpBase op;
	Graph *g;
	Record r;
	RG_Matrix M;                           // Traversed matrix if using the SimpleConsume routine
	int edgesIdx;                          // Edges set by operation
	int srcNodeIdx;                        // Node set by operation
	int destNodeIdx;                       // Node set by operation
	bool expandInto;                       // Both src and dest already resolved
	FT_FilterNode *ft;                     // If not NULL, FilterTree applied to traversed edge
	unsigned int minHops;                  // Maximum number of hops to perform
	unsigned int maxHops;                  // Maximum number of hops to perform
	int edgeRelationCount;                 // Length of edgeRelationTypes
	int *edgeRelationTypes;                // Relation(s) we're traversing
	AlgebraicExpression *ae;               // ArithmeticExpression describing op's traversal pattern
	union {
		AllPathsCtx *allPathsCtx;          // Context for collecting all paths
		AllNeighborsCtx *allNeighborsCtx;  // Context for collecting all neighbors
	};
	bool collect_paths;                    // Whether we must populate the entire path
	GRAPH_EDGE_DIR traverseDir;            // Traverse direction
} RT_CondVarLenTraverse;

RT_OpBase *RT_NewCondVarLenTraverseOp(const RT_ExecutionPlan *plan, AlgebraicExpression *ae, GRAPH_EDGE_DIR traverseDir);

// transform operation from Conditional Variable Length Traverse
// to Expand Into Conditional Variable Length Traverse
void RT_CondVarLenTraverseOp_ExpandInto(RT_CondVarLenTraverse *op);

// Set the FilterTree pointer of a CondVarLenTraverse operation.
void RT_CondVarLenTraverseOp_SetFilter(RT_CondVarLenTraverse *op, FT_FilterNode *ft);

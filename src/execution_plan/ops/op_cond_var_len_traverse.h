/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../algorithms/algorithms.h"
#include "../../arithmetic/algebraic_expression.h"

// OP Traverse
typedef struct {
	OpBase op;
	bool expandInto;                       // both src and dest already resolved
	FT_FilterNode *ft;                     // if not NULL, FilterTree applied to traversed edge
	unsigned int minHops;                  // maximum number of hops to perform
	unsigned int maxHops;                  // maximum number of hops to perform
	int edgeRelationCount;                 // length of edgeRelationTypes
	int *edgeRelationTypes;                // relation(s) we're traversing
	AlgebraicExpression *ae;               // arithmeticExpression describing op's traversal pattern
	bool collect_paths;                    // whether we must populate the entire path
	GRAPH_EDGE_DIR traverseDir;            // traverse direction
} CondVarLenTraverse;

OpBase *NewCondVarLenTraverseOp(const ExecutionPlan *plan, AlgebraicExpression *ae);

// transform operation from Conditional Variable Length Traverse
// to Expand Into Conditional Variable Length Traverse
void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op);

// Set the FilterTree pointer of a CondVarLenTraverse operation.
void CondVarLenTraverseOp_SetFilter(CondVarLenTraverse *op, FT_FilterNode *ft);

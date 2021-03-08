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

/* OP Traverse */
typedef struct {
	OpBase op;
	Graph *g;
	Record r;
	AlgebraicExpression *ae;
	int srcNodeIdx;                 /* Node set by operation. */
	int edgesIdx;                   /* Edges set by operation. */
	int destNodeIdx;                /* Node set by operation. */
	bool expandInto;                /* Both src and dest already resolved. */
	unsigned int minHops;           /* Maximum number of hops to perform. */
	unsigned int maxHops;           /* Maximum number of hops to perform. */
	int edgeRelationCount;          /* Length of edgeRelationTypes. */
	int *edgeRelationTypes;         /* Relation(s) we're traversing. */
	AllPathsCtx *allPathsCtx;
	GRAPH_EDGE_DIR traverseDir;     /* Traverse direction. */
} ShortestPath;

OpBase *NewShortestPathOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae);

/* Transform operation from Conditional Variable Length Traverse
 * to Expand Into Conditional Variable Length Traverse */
void ShortestPathOp_ExpandInto(ShortestPath *op);


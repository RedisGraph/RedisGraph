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
	AlgebraicExpression *ae;
	int srcNodeIdx;                 /* Source node of operation. */
	int pathIdx;                    /* Path constructed by operation. */
	int destNodeIdx;                /* Destination node of operation. */
	bool expandInto;                /* Both src and dest already resolved. */
	TRAVERSE_MODE mode; // TODO maybe delete
	unsigned int minHops;           /* Maximum number of hops to perform. */
	unsigned int maxHops;           /* Maximum number of hops to perform. */
	int edgeRelationCount;          /* Length of edgeRelationTypes. */
	int *edgeRelationTypes;         /* Relation(s) we're traversing. */
	GRAPH_EDGE_DIR traverseDir;     /* Traverse direction. */
} OpShortestPath;

OpBase *NewShortestPathOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae,
						  TRAVERSE_MODE mode);

/* Transform operation from Conditional Variable Length Traverse
 * to Expand Into Conditional Variable Length Traverse */
void ShortestPathOp_ExpandInto(OpShortestPath *op);


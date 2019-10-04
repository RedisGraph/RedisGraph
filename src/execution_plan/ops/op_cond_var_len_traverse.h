/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../graph/graph.h"
#include "../../algorithms/algorithms.h"
#include "../../arithmetic/algebraic_expression.h"

/* OP Traverse */
typedef struct {
	OpBase op;
	Graph *g;
	AlgebraicExpression *ae;
	bool expandInto;                /* Both src and dest already resolved. */
	int srcNodeIdx;                 /* Node set by operation. */
	int destNodeIdx;                /* Node set by operation. */
	int *edgeRelationTypes;         /* Relation(s) we're traversing. */
	int edgeRelationCount;          /* Length of edgeRelationTypes. */
	GRAPH_EDGE_DIR traverseDir;     /* Traverse direction. */
	unsigned int minHops;           /* Maximum number of hops to perform. */
	unsigned int maxHops;           /* Maximum number of hops to perform. */
	AllPathsCtx *allPathsCtx;
	Record r;
} CondVarLenTraverse;

OpBase *NewCondVarLenTraverseOp(Graph *g, RecordMap *record_map, AlgebraicExpression *ae);

/* Transform operation from Conditional Variable Length Traverse
 * to Expand Into Conditional Variable Length Traverse */
void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op);

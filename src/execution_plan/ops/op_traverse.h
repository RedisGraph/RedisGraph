/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../util/vector.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* OP Traverse */
typedef struct {
	OpBase op;
	Graph *graph;
	AlgebraicExpression *algebraic_expression;
	int edgeRelationType;
	Edge *edges;
	GxB_MatrixTupleIter *it;
} Traverse;

/* Creates a new Traverse operation */
OpBase *NewTraverseOp(Graph *g, AlgebraicExpression *ae);

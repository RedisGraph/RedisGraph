/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "execution_plan/ops/op.h"
#include "parser/ast.h"
#include "arithmetic/algebraic_expression.h"
#include "util/vector.h"
#include "GraphBLAS/Include/GraphBLAS.h"

// OP Traverse
typedef struct {
    OpBase op;
    Graph *graph;
    AlgebraicExpression *algebraic_expression;
    int edgeRelationType;
    Edge *edges;    
    GxB_MatrixTupleIter *it;
} Traverse;

// Creates a new Traverse operation
OpBase* NewTraverseOp(Graph *g, AlgebraicExpression *ae);

// TraverseConsume next operation 
// each call will update the graph
// returns NULL when no additional updates are available
Record TraverseConsume(OpBase *opBase);

// Restart iterator
OpResult TraverseReset(OpBase *ctx);

// Frees Traverse
void TraverseFree(OpBase *ctx);

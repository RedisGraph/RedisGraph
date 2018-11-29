/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_COND_TRAVERSE_H
#define __OP_COND_TRAVERSE_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../util/vector.h"

/* OP Traverse */
typedef struct {
    OpBase op;
    Graph *graph;
    AlgebraicExpression *algebraic_expression;
    AlgebraicExpressionResult *algebraic_results;
    GrB_Matrix F;
    GrB_Matrix M;
    int edgeRelationType;
    Edge *edges;
    GxB_MatrixTupleIter *iter;
} CondTraverse;

/* Creates a new Traverse operation */
OpBase* NewCondTraverseOp(Graph *g, AlgebraicExpression *algebraic_expression);

/* TraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult CondTraverseConsume(OpBase *opBase, Record *r);

/* Restart iterator */
OpResult CondTraverseReset(OpBase *ctx);

/* Frees Traverse*/
void CondTraverseFree(OpBase *ctx);

#endif

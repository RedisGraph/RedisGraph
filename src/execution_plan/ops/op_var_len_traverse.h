/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_VAR_LEN_TRAVERSE_H
#define __OP_VAR_LEN_TRAVERSE_H

#include "op.h"
#include "../../graph/graph.h"
#include "../../arithmetic/tuples_iter.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* OP Traverse */
typedef struct {
    OpBase op;
    Graph *g;
    Node **srcNode;                         /* Node set by operation. */
    Node **destNode;                        /* Node set by operation. */
    TuplesIter *it;                         /* Iterator over frontier. */
    GrB_Matrix A;                           /* Matrix we're traversing. */
    GrB_Matrix mask;                        /* Masks reached nodes. */
    GrB_Matrix frontier;                    /* Nodes reached at hop I. */
    GrB_Descriptor desc;                    /* Transpose relation if required. */
    float hops;                             /* Number of hops performed so far. */
    unsigned int minHops;                   /* Maximum number of hops to perform. */
    float maxHops;                          /* Maximum number of hops to perform. */
} VarLenTraverse;

OpBase* NewVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, float maxHops, Graph *g, const QueryGraph *qg);
OpResult VarLenTraverseConsume(OpBase *opBase, QueryGraph* graph);
OpResult VarLenTraverseReset(OpBase *ctx);
void VarLenTraverseFree(OpBase *ctx);
#endif

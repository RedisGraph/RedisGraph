/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_COND_VAR_LEN_TRAVERSE_H
#define __OP_COND_VAR_LEN_TRAVERSE_H

#include "op.h"
#include "../../graph/graph.h"
#include "../../algorithms/algorithms.h"
#include "../../arithmetic/algebraic_expression.h"

/* OP Traverse */
typedef struct {
    OpBase op;
    Graph *g;
    AST *ast;
    AlgebraicExpression *ae;
    bool expandInto;                /* Both src and dest already resolved. */
    int srcNodeIdx;                 /* Node set by operation. */
    int destNodeIdx;                /* Node set by operation. */
    int *relationIDs;               /* Relation(s) we're traversing. */
    int relationIDsCount;           /* Length of relationIDs. */
    GRAPH_EDGE_DIR traverseDir;     /* Traverse direction. */
    unsigned int minHops;           /* Maximum number of hops to perform. */
    unsigned int maxHops;           /* Maximum number of hops to perform. */        
    AllPathsCtx *allPathsCtx;
    Record r;
} CondVarLenTraverse;

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g, AST *ast);
Record CondVarLenTraverseConsume(OpBase *opBase);
OpResult CondVarLenTraverseReset(OpBase *ctx);

/* Transform operation from Conditional Variable Length Traverse
 * to Expand Into Conditional Variable Length Traverse */
void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op);
void CondVarLenTraverseFree(OpBase *ctx);
#endif

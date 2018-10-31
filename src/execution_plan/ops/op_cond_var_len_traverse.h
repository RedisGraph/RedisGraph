/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
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
    const char *srcNodeAlias;       /* Node set by operation. */
    const char *destNodeAlias;      /* Node set by operation. */
    int relationID;                 /* Relation we're traversing. */
    GRAPH_EDGE_DIR traverseDir;     /* Traverse direction. */
    unsigned int minHops;           /* Maximum number of hops to perform. */
    unsigned int maxHops;           /* Maximum number of hops to perform. */    
    size_t pathsCount;              /* Length of Paths. */
    size_t pathsCap;                /* Capacity of Paths. */
    Path *paths;                    /* Array of paths. */
} CondVarLenTraverse;

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g);
OpResult CondVarLenTraverseConsume(OpBase *opBase, Record *r);
OpResult CondVarLenTraverseReset(OpBase *ctx);
void CondVarLenTraverseFree(OpBase *ctx);
#endif

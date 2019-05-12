/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_EXPAND_INTO_H
#define __OP_EXPAND_INTO_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../graph/graph.h"
#include "../../graph/entities/edge.h"
#include "../../arithmetic/algebraic_expression.h"

typedef struct {
    OpBase op;
    Graph *g;                   // Graph context.
    Record r;                   // Current selected record.
    uint srcRecIdx;             // Source node record position.
    uint destRecIdx;            // Destination node record position.
    uint edgeRecIdx;            // Edge record position.
    int relation;               // Edge relation.
    Edge *edges;                // Edges connecting source to destination.
    AlgebraicExpression *exp;   // Expression to evaluate.
} OpExpandInto;

OpBase* NewExpandIntoOp(AlgebraicExpression *exp, uint srcRecIdx, uint destRecIdx, uint edgeRecIdx);
Record OpExpandIntoConsume(OpBase *opBase);
OpResult OpExpandIntoReset(OpBase *ctx);
void OpExpandIntoFree(OpBase *ctx);

#endif

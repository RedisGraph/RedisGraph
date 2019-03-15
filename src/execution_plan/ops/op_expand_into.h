/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_EXPAND_INTO_H
#define __OP_EXPAND_INTO_H

#include "op.h"
#include "../../graph/graphcontext.h"
#include "../../graph/entities/edge.h"
#include "../../parser/ast.h"

typedef struct {
    OpBase op;
    GraphContext *gc;   // Graph context.
    int srcRecIdx;      // Source node record position.
    int destRecIdx;     // Destination node record position.
    Edge *e;            // Edge connecting source to destination.
    Edge **edges;       // Edges connecting source to destination.
} OpExpandInto;

OpBase* NewExpandIntoOp(Node *a, Node *b, Edge *e, AST *ast);
Record OpExpandIntoConsume(OpBase *opBase);
OpResult OpExpandIntoReset(OpBase *ctx);
void OpExpandIntoFree(OpBase *ctx);

#endif

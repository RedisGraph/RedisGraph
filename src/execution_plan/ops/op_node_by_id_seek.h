/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../parser/ast.h"
#include "../../graph/graph.h"

/* Node by ID seek locates an entity by its ID */
typedef struct {
    OpBase op;
    NodeID id;              // Entity ID to seek.
    Graph *g;               // Graph object.
    int nodeRecIdx;         // Position of entity within record.
    int recLength;          // Size of record.
    bool entityRetrieved;   // Did we retrived required entity.
} OpNodeByIdSeek;

OpBase* NewOpNodeByIdSeekOp(const AST *ast, Node *n, NodeID nodeId);
Record OpNodeByIdSeekConsume(OpBase *opBase);
OpResult OpNodeByIdSeekReset(OpBase *ctx);
void OpNodeByIdSeekFree(OpBase *ctx);

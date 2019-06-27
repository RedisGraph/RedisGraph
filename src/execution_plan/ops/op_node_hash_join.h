/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../parser/ast.h"
#include "../../graph/graph.h"
#include "../../index/index.h"
#include "../../graph/entities/node.h"

typedef struct {
    OpBase op;
    const Node *intersect_node;       //
    uint intersect_node_idx;    //
    Record r;
    Record *cached_records;     // Cached records.
    int intersect_idx;        //
    int number_of_intersections;
} OpNodeHashJoin;

/* Creates a new NodeHashJoin operation */
OpBase *NewNodeHashJoin();

/* */
Record NodeHashJoinConsume(OpBase *opBase);

/* */
OpResult NodeHashJoinReset(OpBase *ctx);

/* */
OpResult NodeHashJoinInit(OpBase *ctx);

/* Frees NodeHashJoin */
void NodeHashJoinFree(OpBase *ctx);

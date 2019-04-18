/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../graph/graph.h"

#define ID_RANGE_UNBOUND -1

/* Node by ID seek locates an entity by its ID */
typedef struct {
    OpBase op;
    Graph *g;               // Graph object.
    int nodeRecIdx;         // Position of entity within record.
    int recLength;          // Size of record.
    NodeID minId;           // Min ID to fetch.
    bool minInclusive;      // Include min ID.
    NodeID maxId;           // Max ID to fetch.
    bool maxInclusive;      // Include max ID.
    NodeID currentId;       // Current ID fetched.
} OpNodeByIdSeek;

OpBase* NewOpNodeByIdSeekOp
(
    unsigned int nodeRecIdx,
    NodeID minId,
    NodeID maxId,
    bool includeMin,
    bool includeMax
);

Record OpNodeByIdSeekConsume
(
    OpBase *opBase
);

OpResult OpNodeByIdSeekReset
(
    OpBase *ctx
);

void OpNodeByIdSeekFree
(
    OpBase *ctx
);


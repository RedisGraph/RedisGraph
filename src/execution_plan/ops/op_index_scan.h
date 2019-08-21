/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_INDEX_SCAN_H
#define __OP_INDEX_SCAN_H

#include "op.h"
#include "../../graph/graph.h"
#include "../../index/index.h"
#include "../../../deps/RediSearch/src/redisearch_api.h"

typedef struct {
    OpBase op;
    QGNode *n;
    Graph *g;
    RSIndex *idx;
    uint nodeRecIdx;
    RSResultsIterator *iter;
} IndexScan;

/* Creates a new IndexScan operation */
OpBase *NewIndexScanOp(Graph *g, QGNode *n, uint node_idx, RSIndex *idx, RSResultsIterator *iter);

/* IndexScan next operation
 * called each time a new node is required */
Record IndexScanConsume(OpBase *opBase);

/* Initialize index scan operation. */
OpResult IndexScanInit(OpBase *opBase);

/* Restart iterator */
OpResult IndexScanReset(OpBase *ctx);

/* Frees IndexScan */
void IndexScanFree(OpBase *ctx);

#endif

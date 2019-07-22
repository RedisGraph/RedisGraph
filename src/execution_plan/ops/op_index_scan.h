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
#include "../../graph/entities/node.h"
#include "../../../deps/RediSearch/src/redisearch_api.h"


typedef struct {
    OpBase op;
    Node *n;
    Graph *g;
    uint recLength;  // Number of entries in a record.
    uint nodeRecIdx;
    RSIndex *idx;
    RSResultsIterator *iter;
} IndexScan;

/* Creates a new IndexScan operation */
OpBase *NewIndexScanOp(Graph *g, Node *n, RSIndex *idx, RSResultsIterator *iter, AST *ast);

/* IndexScan next operation
 * called each time a new node is required */
Record IndexScanConsume(OpBase *opBase);

/* Restart iterator */
OpResult IndexScanReset(OpBase *ctx);

/* Frees IndexScan */
void IndexScanFree(OpBase *ctx);

#endif

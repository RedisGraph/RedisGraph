#ifndef __OP_NODE_BY_LABEL_SCAN_H
#define __OP_NODE_BY_LABEL_SCAN_H

#include "op.h"
#include "../../redismodule.h"
#include "../../graph/graph.h"
#include "../../graph/node.h"
#include "../../stores/store.h"
/* NodeByLabelScan 
 * Scans entire label-store
 * Sets node id to current element within
 * the label store */

typedef struct {
    OpBase op;
    Node *node;
    Store *store;
    RedisModuleCtx *ctx;
    RedisModuleString *graph;
    StoreIterator *iter;
} NodeByLabelScan;

/* Creates a new NodeByLabelScan operation */
OpBase* NewNodeByLabelScanOp(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph, char *label);
NodeByLabelScan* NewNodeByLabelScan(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph, char *label);

/* NodeByLabelScan next operation
 * called each time a new ID is required */
OpResult NodeByLabelScanConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult NodeByLabelScanReset(OpBase *ctx);

/* Frees NodeByLabelScan */
void NodeByLabelScanFree(OpBase *ctx);

#endif
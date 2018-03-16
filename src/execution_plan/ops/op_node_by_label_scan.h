#ifndef __OP_NODE_BY_LABEL_SCAN_H
#define __OP_NODE_BY_LABEL_SCAN_H

#include "op.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../graph/node.h"
#include "../../stores/store.h"
/* NodeByLabelScan 
 * Scans entire label-store
 * Sets node id to current element within
 * the label store */

typedef struct {
    OpBase op;
    Node **node;            /* node being scanned */
    Node *_node;
    LabelStore *store;
    RedisModuleCtx *ctx;
    const char *graph;      /* queried graph id */
    LabelStoreIterator *iter;
} NodeByLabelScan;

/* Creates a new NodeByLabelScan operation */
OpBase *NewNodeByLabelScanOp(RedisModuleCtx *ctx, QueryGraph *g, Node **node,
                            const char *graph_name, char *label);

NodeByLabelScan* NewNodeByLabelScan(RedisModuleCtx *ctx, QueryGraph *g, Node **node,
                                    const char *graph_name, char *label);

/* NodeByLabelScan next operation
 * called each time a new ID is required */
OpResult NodeByLabelScanConsume(OpBase *opBase, QueryGraph* graph);

/* Restart iterator */
OpResult NodeByLabelScanReset(OpBase *ctx);

/* Frees NodeByLabelScan */
void NodeByLabelScanFree(OpBase *ctx);

#endif
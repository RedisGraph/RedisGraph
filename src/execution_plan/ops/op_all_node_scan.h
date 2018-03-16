#ifndef __OP_ALL_NODE_SCAN_H__
#define __OP_ALL_NODE_SCAN_H__

#include "op.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../graph/node.h"
#include "../../stores/store.h"

/* AllNodesScan
 * Scans entire graph
 * Sets node id to current element */

 typedef struct {
    OpBase op;
    Node **node;            /* node being scanned */
    Node *_node;
    LabelStore *store;           /* store being scanned */
    RedisModuleCtx *ctx;    /* redis module API context */
    const char *graph;      /* queried graph id */
    LabelStoreIterator *iter;    /* graph iterator */
 } AllNodeScan;

OpBase* NewAllNodeScanOp(RedisModuleCtx *ctx, QueryGraph *g, Node **n, const char *graph_name);
AllNodeScan* NewAllNodeScan(RedisModuleCtx *ctx, QueryGraph *g, Node **n, const char *graph_name);
OpResult AllNodeScanConsume(OpBase *opBase, QueryGraph* graph);
OpResult AllNodeScanReset(OpBase *op);
void AllNodeScanFree(OpBase *ctx);

#endif
#ifndef __OP_ALL_NODE_SCAN_H__
#define __OP_ALL_NODE_SCAN_H__

#include "op.h"
#include "../../redismodule.h"
#include "../../graph/graph.h"
#include "../../graph/node.h"
#include "../../stores/store.h"

/* AllNodesScan
 * Scans entire graph
 * Sets node id to current element */

 typedef struct {
     OpBase op;
     Node *node;                // node being scanned
     RedisModuleCtx *ctx;       // redis module API context
     RedisModuleString *graph;  // queried graph id
     StoreIterator *iter;       // graph iterator
 } AllNodeScan;

OpBase* NewAllNodeScanOp(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph);
AllNodeScan* NewAllNodeScan(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph);
OpResult AllNodeScanConsume(OpBase *opBase, Graph* graph);
OpResult AllNodeScanReset(OpBase *op);
void AllNodeScanFree(AllNodeScan *allNodeScan);

#endif
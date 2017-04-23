#include "all_node_scan.h"

OpBase* NewAllNodeScanOp(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph) {
    return (OpBase*)NewAllNodeScan(ctx, node, graph);
}

AllNodeScan* NewAllNodeScan(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->ctx = ctx;
    allNodeScan->node = node;
    allNodeScan->graph = graph;
    allNodeScan->iter = NULL;

    // Set our Op operations
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.next = AllNodeScanConsume;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.free = AllNodeScanFree;

    return allNodeScan;
}

OpResult AllNodeScanConsume(OpBase *opBase, Graph* graph) {
    AllNodeScan *op = (AllNodeScan*)opBase;
    
    if(op->iter == NULL) {
        return OP_DEPLETED;
    }

    char *id = StoreIterator_Next(op->iter);
    
    if(id == NULL) {
        return OP_DEPLETED;
    }

    // Set node's ID.
    op->node->id = id;
    return OP_OK;
}

OpResult AllNodeScanReset(OpBase *op) {
    AllNodeScan *allNodeScan = (AllNodeScan*)op;

    // Free previous iterator.
    if(allNodeScan->iter != NULL) {
       StoreIterator_Free(allNodeScan->iter);
    }

    // Get graph store
    Store *store = GetStore(allNodeScan->ctx, STORE_NODE, allNodeScan->graph, NULL);
    if(store == NULL) {
        return OP_ERR;
    }

    // Get entire graph cursor
    allNodeScan->iter = Store_Search(store, "");
    if(allNodeScan->iter == NULL) {
        return OP_ERR;
    }

    return OP_OK;
}

void AllNodeScanFree(AllNodeScan *allNodeScan) {
    if(allNodeScan->iter != NULL) {
       StoreIterator_Free(allNodeScan->iter);
    }
    free(allNodeScan);
}
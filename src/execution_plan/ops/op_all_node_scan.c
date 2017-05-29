#include "op_all_node_scan.h"

OpBase* NewAllNodeScanOp(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph) {
    return (OpBase*)NewAllNodeScan(ctx, node, graph);
}

AllNodeScan* NewAllNodeScan(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph) {
    // Get graph store
    Store *store = GetStore(ctx, STORE_NODE, graph, NULL);
    if(store == NULL) {
        return NULL;
    }

    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->ctx = ctx;
    allNodeScan->node = node;
    allNodeScan->graph = graph;
    allNodeScan->iter = Store_Search(store, "");

    // Set our Op operations
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.next = AllNodeScanConsume;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.free = AllNodeScanFree;
    allNodeScan->op.modifies = NewVector(char*, 1);
    
    Vector_Push(allNodeScan->op.modifies, node->alias);

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
    if(op->node->id != NULL) {
        free(op->node->id);
    }
    
    op->node->id = strdup(id);

    return OP_OK;
}

OpResult AllNodeScanReset(OpBase *op) {
    AllNodeScan *allNodeScan = op;
    
    if(allNodeScan->iter != NULL) {
       StoreIterator_Free(allNodeScan->iter);
    }

    Store *store = GetStore(allNodeScan->ctx, STORE_NODE, allNodeScan->graph, NULL);
    allNodeScan->iter = Store_Search(store, "");
    return OP_OK;
}

void AllNodeScanFree(AllNodeScan *allNodeScan) {
    if(allNodeScan->iter != NULL) {
       StoreIterator_Free(allNodeScan->iter);
    }
    free(allNodeScan);
}
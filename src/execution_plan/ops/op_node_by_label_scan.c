#include "op_node_by_label_scan.h"

OpBase *NewNodeByLabelScanOp(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph, char *label) {
    return (OpBase*)NewNodeByLabelScan(ctx, node, graph, label);
}

NodeByLabelScan* NewNodeByLabelScan(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph, char *label) {
    // Get graph store
    RedisModuleString *rmsLabel = RedisModule_CreateString(ctx, label, strlen(label));
    Store *store = GetStore(ctx, STORE_NODE, graph, rmsLabel);
    RedisModule_FreeString(ctx, rmsLabel);
    if(store == NULL) {
        return NULL;
    }
    
    NodeByLabelScan *nodeByLabelScan = malloc(sizeof(NodeByLabelScan));
    nodeByLabelScan->ctx = ctx;
    nodeByLabelScan->node = node;
    nodeByLabelScan->graph = graph;
    nodeByLabelScan->store = store;
    nodeByLabelScan->iter = Store_Search(store, "");
    

    // Set our Op operations
    nodeByLabelScan->op.name = "Node By Label Scan";
    nodeByLabelScan->op.next = NodeByLabelScanConsume;
    nodeByLabelScan->op.reset = NodeByLabelScanReset;
    nodeByLabelScan->op.free = NodeByLabelScanFree;

    return nodeByLabelScan;
}

OpResult NodeByLabelScanConsume(OpBase *opBase, Graph* graph) {
    NodeByLabelScan *op = (NodeByLabelScan*)opBase;

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

OpResult NodeByLabelScanReset(OpBase *ctx) {
    NodeByLabelScan *nodeByLabelScan = ctx;
    if(nodeByLabelScan->iter != NULL) {
        StoreIterator_Free(nodeByLabelScan->iter);
    }
    
    nodeByLabelScan->iter = Store_Search(nodeByLabelScan->store, "");
    return OP_OK;
}

void NodeByLabelScanFree(OpBase *op) {
    NodeByLabelScan *nodeByLabelScan = op;
    if(nodeByLabelScan->iter != NULL) {
        StoreIterator_Free(nodeByLabelScan->iter);
    }
    free(nodeByLabelScan);
}
#include "node_by_label_scan.h"

OpResult NodeByLabelScanConsume(OpBase *opBase, Graph* graph) {
    NodeByLabelScan *op = (NodeByLabelScan*)opBase;

    if(op->iter == NULL) {
        return OP_DEPLETED;
    }

    char *id = StoreIterator_Next(op->iter);

    if(id == NULL) {
        return OP_DEPLETED;
    }
    
    // Set scanned node id
    op->node->id = id;
    return OP_OK;
}

OpResult NodeByLabelScanReset(OpBase *ctx) {
    NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan*)ctx;
    
    // Free old iterator
    if(nodeByLabelScan->iter != NULL) {
        StoreIterator_Free(nodeByLabelScan->iter);
    }

    // Create a new iterator.
    // Get graph store
    Store *store = GetStore(nodeByLabelScan->ctx, STORE_NODE, nodeByLabelScan->graph, nodeByLabelScan->label);
    if(store == NULL) {
        return OP_ERR;
    }

    nodeByLabelScan->iter = Store_Search(store, "");
    if(nodeByLabelScan->iter == NULL) {
        return OP_ERR;
    }

    return OP_OK;
}

OpBase *NewNodeByLabelScanOp(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph, char *label) {
    return (OpBase*)NewNodeByLabelScan(ctx, node, graph, label);
}

NodeByLabelScan* NewNodeByLabelScan(RedisModuleCtx *ctx, Node *node, RedisModuleString *graph, char *label) {
    NodeByLabelScan *nodeByLabelScan = malloc(sizeof(NodeByLabelScan));
    nodeByLabelScan->ctx = ctx;
    nodeByLabelScan->node = node;
    nodeByLabelScan->graph = graph;
    nodeByLabelScan->label = RedisModule_CreateString(ctx, label, strlen(label));

    // Set our Op operations
    nodeByLabelScan->op.name = "Node By Label Scan";
    nodeByLabelScan->op.next = NodeByLabelScanConsume;
    nodeByLabelScan->op.reset = NodeByLabelScanReset;
    nodeByLabelScan->op.free = NodeByLabelScanFree;

    return nodeByLabelScan;
}

void NodeByLabelScanFree(NodeByLabelScan *nodeByLabelScan) {
    RedisModule_FreeString(nodeByLabelScan->ctx, nodeByLabelScan->label);
    if(nodeByLabelScan->iter != NULL) {
        StoreIterator_Free(nodeByLabelScan->iter);
    }
    free(nodeByLabelScan);
}
#include "op_node_by_label_scan.h"

OpBase *NewNodeByLabelScanOp(RedisModuleCtx *ctx, Graph *g, Node **node,
                            const char *graph_name, char *label) {
    return (OpBase*)NewNodeByLabelScan(ctx, g, node, graph_name, label);
}

NodeByLabelScan* NewNodeByLabelScan(RedisModuleCtx *ctx, Graph *g, Node **node,
                                    const char *graph_name, char *label) {
    // Get graph store
    Store *store = GetStore(ctx, STORE_NODE, graph_name, label);
    if(store == NULL) {
        return NULL;
    }
    
    NodeByLabelScan *nodeByLabelScan = malloc(sizeof(NodeByLabelScan));
    nodeByLabelScan->ctx = ctx;
    nodeByLabelScan->node = node;
    nodeByLabelScan->_node = *node;
    nodeByLabelScan->graph = graph_name;
    nodeByLabelScan->store = store;
    nodeByLabelScan->iter = Store_Search(store, "");
    

    // Set our Op operations
    nodeByLabelScan->op.name = "Node By Label Scan";
    nodeByLabelScan->op.type = OPType_NODE_BY_LABEL_SCAN;
    nodeByLabelScan->op.consume = NodeByLabelScanConsume;
    nodeByLabelScan->op.reset = NodeByLabelScanReset;
    nodeByLabelScan->op.free = NodeByLabelScanFree;
    nodeByLabelScan->op.modifies = NewVector(char*, 1);
    
    Vector_Push(nodeByLabelScan->op.modifies, Graph_GetNodeAlias(g, *node));
    
    return nodeByLabelScan;
}

OpResult NodeByLabelScanConsume(OpBase *opBase, Graph* graph) {
    NodeByLabelScan *op = (NodeByLabelScan*)opBase;

    if(op->iter == NULL) return OP_DEPLETED;

    char *id;
    tm_len_t idLen;
    
    /* Update node */
    Node **n = op->node;
    int res = StoreIterator_Next(op->iter, &id, &idLen, (void**)op->node);

    if(res == 0) {
        return OP_DEPLETED;
    } 

    return OP_OK;
}

OpResult NodeByLabelScanReset(OpBase *ctx) {
    NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan*)ctx;

    /* Restore original node. */
    *nodeByLabelScan->node = nodeByLabelScan->_node;
    
    if(nodeByLabelScan->iter != NULL) {
        StoreIterator_Free(nodeByLabelScan->iter);
    }
    
    nodeByLabelScan->iter = Store_Search(nodeByLabelScan->store, "");
    return OP_OK;
}

void NodeByLabelScanFree(OpBase *op) {
    NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan*)op;
    if(nodeByLabelScan->iter != NULL) {
        StoreIterator_Free(nodeByLabelScan->iter);
    }
    free(nodeByLabelScan);
}
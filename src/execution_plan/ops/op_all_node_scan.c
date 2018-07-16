/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_all_node_scan.h"

OpBase* NewAllNodeScanOp(QueryGraph *qg, const Graph *g, Node **n) {
    return (OpBase*)NewAllNodeScan(qg, g, n);
}

AllNodeScan* NewAllNodeScan(QueryGraph *qg, const Graph *g, Node **n) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->node = n;
    allNodeScan->_node = *n;
    allNodeScan->iter = Graph_ScanNodes(g);

    // Set our Op operations
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.consume = AllNodeScanConsume;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.free = AllNodeScanFree;
    allNodeScan->op.modifies = NewVector(char*, 1);
    
    Vector_Push(allNodeScan->op.modifies, QueryGraph_GetNodeAlias(qg, *n));

    return allNodeScan;
}

OpResult AllNodeScanConsume(OpBase *opBase, QueryGraph* graph) {
    AllNodeScan *op = (AllNodeScan*)opBase;
    
    Node *node = NodeIterator_Next(op->iter);
    if(node == NULL) {
        return OP_DEPLETED;
    }

    *op->node = node;

    return OP_OK;
}

OpResult AllNodeScanReset(OpBase *op) {
    AllNodeScan *allNodeScan = (AllNodeScan*)op;
    
    *allNodeScan->node = allNodeScan->_node;
    NodeIterator_Reset(allNodeScan->iter);
    
    return OP_OK;
}

void AllNodeScanFree(OpBase *ctx) {
    AllNodeScan *op = (AllNodeScan *)ctx;
    NodeIterator_Free(op->iter);
    Vector_Free(op->op.modifies);
    free(op);
}

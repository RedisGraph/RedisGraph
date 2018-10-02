/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_all_node_scan.h"

OpBase* NewAllNodeScanOp(const Graph *g, Node *n) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->node = n;
    allNodeScan->iter = Graph_ScanNodes(g);

    // Set our Op operations
    OpBase_Init(&allNodeScan->op);
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.consume = AllNodeScanConsume;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.free = AllNodeScanFree;
    allNodeScan->op.modifies = NewVector(char*, 1);

    Vector_Push(allNodeScan->op.modifies, n->alias);

    return (OpBase*)allNodeScan;
}

OpResult AllNodeScanConsume(OpBase *opBase, Record *r) {
    AllNodeScan *op = (AllNodeScan*)opBase;
    
    NodeID id = DataBlockIterator_Position(op->iter);
    Node *node = (Node*)DataBlockIterator_Next(op->iter);
    if(node == NULL) return OP_DEPLETED;

    node->id = id;
    Record_AddEntry(r, op->node->alias, SI_PtrVal(node));

    return OP_OK;
}

OpResult AllNodeScanReset(OpBase *op) {
    AllNodeScan *allNodeScan = (AllNodeScan*)op;
    DataBlockIterator_Reset(allNodeScan->iter);
    
    return OP_OK;
}

void AllNodeScanFree(OpBase *ctx) {
    AllNodeScan *op = (AllNodeScan *)ctx;    
    DataBlockIterator_Free(op->iter);
    Vector_Free(op->op.modifies);
}

/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../parser/newast.h"

OpBase* NewAllNodeScanOp(const Graph *g, Node *n, uint node_idx) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->iter = Graph_ScanNodes(g);

    NEWAST *ast = NEWAST_GetFromTLS();
    allNodeScan->nodeRecIdx = node_idx;
    allNodeScan->recLength = NEWAST_RecordLength(ast);

    // Set our Op operations
    OpBase_Init(&allNodeScan->op);
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.consume = AllNodeScanConsume;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.free = AllNodeScanFree;
    allNodeScan->op.modifies = NewVector(char*, 1);

    if (n->alias) Vector_Push(allNodeScan->op.modifies, n->alias);

    return (OpBase*)allNodeScan;
}

Record AllNodeScanConsume(OpBase *opBase) {
    AllNodeScan *op = (AllNodeScan*)opBase;

    Entity *en = (Entity*)DataBlockIterator_Next(op->iter);
    if(en == NULL) return NULL;
    
    Record r = Record_New(op->recLength);
    Node *n = Record_GetNode(r, op->nodeRecIdx);
    n->entity = en;

    return r;
}

OpResult AllNodeScanReset(OpBase *op) {
    AllNodeScan *allNodeScan = (AllNodeScan*)op;
    DataBlockIterator_Reset(allNodeScan->iter);
    return OP_OK;
}

void AllNodeScanFree(OpBase *ctx) {
    AllNodeScan *op = (AllNodeScan *)ctx;    
    DataBlockIterator_Free(op->iter);
}

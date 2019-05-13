/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../parser/ast.h"

OpBase* NewAllNodeScanOp(const Graph *g, Node *n, uint node_idx) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->iter = Graph_ScanNodes(g);

    AST *ast = AST_GetFromTLS();
    allNodeScan->nodeRecIdx = node_idx;

    // Set our Op operations
    OpBase_Init(&allNodeScan->op);
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.consume = AllNodeScanConsume;
    allNodeScan->op.init = AllNodeScanInit;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.free = AllNodeScanFree;

    allNodeScan->op.modifies = array_new(uint, 1);
    allNodeScan->op.modifies = array_append(allNodeScan->op.modifies, node_idx);

    return (OpBase*)allNodeScan;
}

OpResult AllNodeScanInit(OpBase *opBase) {
    AllNodeScan *op = (AllNodeScan*)opBase;
    AST *ast = AST_GetFromTLS();
    op->recLength = AST_RecordLength(ast);
    return OP_OK;
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

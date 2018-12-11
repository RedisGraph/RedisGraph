/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_all_node_scan.h"
#include "../../parser/ast.h"

OpBase* NewAllNodeScanOp(const Graph *g, Node *n) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->iter = Graph_ScanNodes(g);

    AST *ast = AST_GetFromLTS();
    allNodeScan->nodeRecIdx = AST_GetAliasID(ast, n->alias);

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

OpResult AllNodeScanConsume(OpBase *opBase, Record r) {
    AllNodeScan *op = (AllNodeScan*)opBase;

    Entity *en = (Entity*)DataBlockIterator_Next(op->iter);
    if(en == NULL) return OP_DEPLETED;
    
    Node *n = Record_GetNode(r, op->nodeRecIdx);
    n->entity = en;

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
}

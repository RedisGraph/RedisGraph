/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../parser/ast.h"

int AllNodeScanToString(const OpBase *ctx, char *buff, uint buff_len) {
    const AllNodeScan *op = (const AllNodeScan*)ctx;
    int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
    offset += Node_ToString(op->n, buff + offset, buff_len - offset);
    return offset;
}

OpBase* NewAllNodeScanOp(const Graph *g, Node *n, AST *ast) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->n = n;
    allNodeScan->iter = Graph_ScanNodes(g);
    allNodeScan->nodeRecIdx = AST_GetAliasID(ast, n->alias);
    allNodeScan->recLength = AST_AliasCount(ast);

    // Set our Op operations
    OpBase_Init(&allNodeScan->op);
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.consume = AllNodeScanConsume;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.toString = AllNodeScanToString;
    allNodeScan->op.free = AllNodeScanFree;
    allNodeScan->op.modifies = NewVector(char*, 1);

    Vector_Push(allNodeScan->op.modifies, n->alias);

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

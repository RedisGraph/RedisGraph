/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../ast/ast.h"

int AllNodeScanToString(const OpBase *ctx, char *buff, uint buff_len) {
    const AllNodeScan *op = (const AllNodeScan*)ctx;
    int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
    offset += QGNode_ToString(op->n, buff + offset, buff_len - offset);
    return offset;
}

OpBase* NewAllNodeScanOp(const Graph *g, QGNode *n, uint node_idx) {
    AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
    allNodeScan->n = n;
    allNodeScan->iter = Graph_ScanNodes(g);
    allNodeScan->nodeRecIdx = node_idx;

    // Set our Op operations
    OpBase_Init(&allNodeScan->op);
    allNodeScan->op.name = "All Node Scan";
    allNodeScan->op.type = OPType_ALL_NODE_SCAN;
    allNodeScan->op.consume = AllNodeScanConsume;
    allNodeScan->op.init = AllNodeScanInit;
    allNodeScan->op.reset = AllNodeScanReset;
    allNodeScan->op.toString = AllNodeScanToString;
    allNodeScan->op.free = AllNodeScanFree;

    allNodeScan->op.modifies = array_new(uint, 1);
    allNodeScan->op.modifies = array_append(allNodeScan->op.modifies, node_idx);

    return (OpBase*)allNodeScan;
}

OpResult AllNodeScanInit(OpBase *opBase) {
    return OP_OK;
}

Record AllNodeScanConsume(OpBase *opBase) {
    AllNodeScan *op = (AllNodeScan*)opBase;

    Entity *en = (Entity*)DataBlockIterator_Next(op->iter);
    if(en == NULL) return NULL;
    
    Record r = Record_New(opBase->record_map->record_len);
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

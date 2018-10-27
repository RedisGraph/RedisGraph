/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_node_by_label_scan.h"

OpBase *NewNodeByLabelScanOp(GraphContext *gc, Node *node) {
    NodeByLabelScan *nodeByLabelScan = malloc(sizeof(NodeByLabelScan));
    nodeByLabelScan->g = gc->g;
    nodeByLabelScan->node = node;
    nodeByLabelScan->_zero_matrix = NULL;

    /* Retrieve label matrix. */
    GrB_Matrix label_matrix = GraphContext_GetMatrix(gc, node->label, STORE_NODE);
    if (label_matrix) {
        nodeByLabelScan->iter = TuplesIter_new(label_matrix);
    } else {
        /* Label does not exist, use a fake empty matrix. */
        GrB_Matrix_new(&nodeByLabelScan->_zero_matrix, GrB_BOOL, 1, 1);
        nodeByLabelScan->iter = TuplesIter_new(nodeByLabelScan->_zero_matrix);
    }

    // Set our Op operations
    OpBase_Init(&nodeByLabelScan->op);
    nodeByLabelScan->op.name = "Node By Label Scan";
    nodeByLabelScan->op.type = OPType_NODE_BY_LABEL_SCAN;
    nodeByLabelScan->op.consume = NodeByLabelScanConsume;
    nodeByLabelScan->op.reset = NodeByLabelScanReset;
    nodeByLabelScan->op.free = NodeByLabelScanFree;
    
    nodeByLabelScan->op.modifies = NewVector(char*, 1);
    Vector_Push(nodeByLabelScan->op.modifies, node->alias);

    return (OpBase*)nodeByLabelScan;
}

OpResult NodeByLabelScanConsume(OpBase *opBase, Record *r) {
    NodeByLabelScan *op = (NodeByLabelScan*)opBase;

    GrB_Index nodeId;

    if(TuplesIter_next(op->iter, NULL, &nodeId) == TuplesIter_DEPLETED) {
        return OP_DEPLETED;
    }

    Node *n = Graph_GetNode(op->g, nodeId);
    Record_AddEntry(r, op->node->alias, SI_PtrVal(n));

    return OP_OK;
}

OpResult NodeByLabelScanReset(OpBase *ctx) {
    NodeByLabelScan *op = (NodeByLabelScan*)ctx;
    TuplesIter_reset(op->iter);
    return OP_OK;
}

void NodeByLabelScanFree(OpBase *op) {
    NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan*)op;
    TuplesIter_free(nodeByLabelScan->iter);
    
    if(nodeByLabelScan->_zero_matrix != NULL) {
        GrB_Matrix_free(&nodeByLabelScan->_zero_matrix);
    }
}

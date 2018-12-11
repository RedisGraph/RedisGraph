/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_NODE_BY_LABEL_SCAN_H
#define __OP_NODE_BY_LABEL_SCAN_H

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../graph/graph.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* NodeByLabelScan, scans entire label. */

typedef struct {
    OpBase op;
    Node *node;                 /* Node being scanned. */
    int node_rec_idx;           /* Node position within record. */
    Graph *g;
    GxB_MatrixTupleIter *iter;
    GrB_Matrix _zero_matrix;    /* Fake matrix, in-case label does not exists. */
} NodeByLabelScan;

/* Creates a new NodeByLabelScan operation */
OpBase *NewNodeByLabelScanOp(GraphContext *gc, Node *node);

/* NodeByLabelScan next operation
 * called each time a new ID is required */
OpResult NodeByLabelScanConsume(OpBase *opBase, Record r);

/* Restart iterator */
OpResult NodeByLabelScanReset(OpBase *ctx);

/* Frees NodeByLabelScan */
void NodeByLabelScanFree(OpBase *ctx);

#endif

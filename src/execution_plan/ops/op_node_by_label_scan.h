/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_NODE_BY_LABEL_SCAN_H
#define __OP_NODE_BY_LABEL_SCAN_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../graph/graph.h"
#include "../../graph/entities/node.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* NodeByLabelScan, scans entire label. */

typedef struct {
    OpBase op;
    Node *node;                 /* Node being scanned. */
    unsigned int nodeRecIdx;    /* Node position within record. */
    unsigned int recLength;     /* Number of entries in a record. */
    Graph *g;
    GxB_MatrixTupleIter *iter;
    GrB_Matrix _zero_matrix;    /* Fake matrix, in-case label does not exists. */
} NodeByLabelScan;

/* Creates a new NodeByLabelScan operation */
OpBase *NewNodeByLabelScanOp(GraphContext *gc, Node *node);

/* NodeByLabelScan next operation
 * called each time a new ID is required */
Record NodeByLabelScanConsume(OpBase *opBase);

/* Restart iterator */
OpResult NodeByLabelScanReset(OpBase *ctx);

/* Frees NodeByLabelScan */
void NodeByLabelScanFree(OpBase *ctx);

#endif

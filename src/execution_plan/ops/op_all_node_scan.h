/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_ALL_NODE_SCAN_H__
#define __OP_ALL_NODE_SCAN_H__

#include "op.h"
#include "../../parser/ast.h"
#include "../../graph/graph.h"
#include "../../graph/query_graph.h"
#include "../../graph/entities/node.h"
#include "../../util/datablock/datablock_iterator.h"

/* AllNodesScan
 * Scans entire graph */
 typedef struct {
    OpBase op;
    DataBlockIterator *iter;
    uint nodeRecIdx;
    uint recLength;  // Number of entries in a record.
 } AllNodeScan;

OpBase* NewAllNodeScanOp(const Graph *g, Node *n);
Record AllNodeScanConsume(OpBase *opBase);
OpResult AllNodeScanReset(OpBase *op);
void AllNodeScanFree(OpBase *ctx);

#endif

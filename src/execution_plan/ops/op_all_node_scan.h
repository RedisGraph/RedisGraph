/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_ALL_NODE_SCAN_H__
#define __OP_ALL_NODE_SCAN_H__

#include "op.h"
#include "../../graph/query_graph.h"
#include "../../graph/entities/node.h"
#include "../../graph/graph.h"
#include "../../util/datablock/datablock_iterator.h"

/* AllNodesScan
 * Scans entire graph */

 typedef struct {
    OpBase op;
    Node *node;
    DataBlockIterator *iter;
 } AllNodeScan;

OpBase* NewAllNodeScanOp(const Graph *g, Node *n);
OpResult AllNodeScanConsume(OpBase *opBase, Record *r);
OpResult AllNodeScanReset(OpBase *op);
void AllNodeScanFree(OpBase *ctx);

#endif

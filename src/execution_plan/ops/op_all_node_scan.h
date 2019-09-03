/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_ALL_NODE_SCAN_H__
#define __OP_ALL_NODE_SCAN_H__

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../graph/query_graph.h"
#include "../../graph/entities/node.h"
#include "../../util/datablock/datablock_iterator.h"

/* AllNodesScan
 * Scans entire graph */
typedef struct {
	OpBase op;
	QGNode *n;
	DataBlockIterator *iter;
	uint nodeRecIdx;
} AllNodeScan;

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const Graph *g, QGNode *n);
Record AllNodeScanConsume(OpBase *opBase);
OpResult AllNodeScanInit(OpBase *opBase);
OpResult AllNodeScanReset(OpBase *op);
void AllNodeScanFree(OpBase *ctx);

#endif

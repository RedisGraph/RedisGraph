/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

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
	const QGNode *n;
    uint nodeRecIdx;
	DataBlockIterator *iter;
} AllNodeScan;

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const Graph *g, const QGNode *n);

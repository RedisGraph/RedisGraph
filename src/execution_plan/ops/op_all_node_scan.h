/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
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

OpBase *NewAllNodeScanOp(const Graph *g, QGNode *n, uint rec_idx);

/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../IR/execution_plan/execution_plan.h"
#include "../../storage/graph/graph.h"
#include "../../IR/query_graph/query_graph.h"
#include "../../storage/entities/node.h"
#include "../../storage/datablock/datablock_iterator.h"

/* AllNodesScan
 * Scans entire graph */
typedef struct {
	OpBase op;
	const char *alias;          /* Alias of the node being scanned by this op. */
	uint nodeRecIdx;
	DataBlockIterator *iter;
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} AllNodeScan;

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const char *alias);


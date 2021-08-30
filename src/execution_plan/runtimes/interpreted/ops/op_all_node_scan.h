/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_all_node_scan.h"
#include "../../../../util/datablock/datablock_iterator.h"

/* AllNodesScan
 * Scans entire graph */
typedef struct {
	RT_OpBase op;
	const AllNodeScan *op_desc;
	uint nodeRecIdx;
	DataBlockIterator *iter;
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} RT_AllNodeScan;

RT_OpBase *RT_NewAllNodeScanOp(const RT_ExecutionPlan *plan, const AllNodeScan *op_desc);

/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../../util/datablock/datablock_iterator.h"

/* AllNodesScan
 * Scans entire graph */
typedef struct {
	RT_OpBase op;
	const char *alias;          /* Alias of the node being scanned by this op. */
	uint nodeRecIdx;
	DataBlockIterator *iter;
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} RT_AllNodeScan;

RT_OpBase *RT_NewAllNodeScanOp(const RT_ExecutionPlan *plan, const char *alias);


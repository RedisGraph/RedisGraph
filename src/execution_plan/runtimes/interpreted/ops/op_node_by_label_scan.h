/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../ops/shared/scan_functions.h"
#include "../runtime_execution_plan.h"
#include "../../../../graph/graph.h"
#include "../../../../util/range/unsigned_range.h"
#include "../../../../graph/rg_matrix/rg_matrix_iter.h"

/* NodeByLabelScan, scans entire label. */

typedef struct {
	RT_OpBase op;
	Graph *g;
	NodeScanCtx n;              // Label data of node being scanned
	unsigned int nodeRecIdx;    // Node position within record
	UnsignedRange *id_range;    // ID range to iterate over
	RG_MatrixTupleIter *iter;
	Record child_record;        // The Record this op acts on if it is not a tap
} RT_NodeByLabelScan;

// Creates a new NodeByLabelScan operation
RT_OpBase *RT_NewNodeByLabelScanOp(const RT_ExecutionPlan *plan, NodeScanCtx n);

// Transform a simple label scan to perform additional range query over the label  matrix
void RT_NodeByLabelScanOp_SetIDRange(RT_NodeByLabelScan *op, UnsignedRange *id_range);


/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../../graph/graph.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_node_by_label_scan.h"
#include "../../../ops/shared/scan_functions.h"
#include "../../../../util/range/unsigned_range.h"
#include "../../../../graph/rg_matrix/rg_matrix_iter.h"

/* NodeByLabelScan, scans entire label. */

typedef struct {
	RT_OpBase op;
	const NodeByLabelScan *op_desc;
	Graph *g;
	unsigned int nodeRecIdx;    // Node position within record
	RG_MatrixTupleIter *iter;
	Record child_record;        // The Record this op acts on if it is not a tap
} RT_NodeByLabelScan;

// Creates a new NodeByLabelScan operation
RT_OpBase *RT_NewNodeByLabelScanOp(const RT_ExecutionPlan *plan, const NodeByLabelScan *op_desc);

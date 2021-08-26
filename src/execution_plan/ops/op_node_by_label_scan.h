/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "shared/scan_functions.h"
#include "../execution_plan.h"
#include "../../util/range/unsigned_range.h"

// NodeByLabelScan, scans entire label
typedef struct {
	OpBase op;
	NodeScanCtx n;              // Label data of node being scanned
	UnsignedRange *id_range;    // ID range to iterate over
} NodeByLabelScan;

// Creates a new NodeByLabelScan operation
OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, NodeScanCtx n);

// Transform a simple label scan to perform additional range query over the label  matrix
void NodeByLabelScanOp_SetIDRange(NodeByLabelScan *op, UnsignedRange *id_range);

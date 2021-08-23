/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"

// AllNodesScan
// Scans entire graph
typedef struct {
	OpBase op;
	const char *alias; // Alias of the node being scanned by this op
} AllNodeScan;

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const char *alias);

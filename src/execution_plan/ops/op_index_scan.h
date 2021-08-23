/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../index/index.h"
#include "shared/scan_functions.h"
#include "redisearch_api.h"

typedef struct {
	OpBase op;
	NodeScanCtx n;                      // label data of node being scanned
	FT_FilterNode *filter;              // filter from which to compose index query
} IndexScan;

// creates a new IndexScan operation
OpBase *NewIndexScanOp(const ExecutionPlan *plan, NodeScanCtx n,
	FT_FilterNode *filter);


/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
	Graph *g;
	RSIndex *idx;
	const FT_FilterNode *filter;
	NodeScanCtx n;             // label data of node being scanned
	uint nodeRecIdx;           // index of the node being scanned in the Record
	RSQNode *rs_query_node;    // rediSearch query node used to construct iterator
	RSResultsIterator *iter;   // rediSearch iterator over an index with the appropriate filters
	Record child_record;       // the Record this op acts on if it is not a tap
} IndexScan;

// creates a new IndexScan operation
OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, NodeScanCtx n, RSIndex *idx,
					   const FT_FilterNode *filter);


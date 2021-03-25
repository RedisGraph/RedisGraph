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
	Graph *g;
	bool rebuild_index_query;  // should we rebuild RediSearch index query for each input record
	RSIndex *idx;              // index to query
	NodeScanCtx n;             // label data of node being scanned
	uint nodeRecIdx;           // index of the node being scanned in the Record
	RSResultsIterator *iter;   // rediSearch iterator over an index with the appropriate filters
	FT_FilterNode *filter;     // filters from which to compose index query
	Record child_record;       // the Record this op acts on if it is not a tap
} IndexScan;

// creates a new IndexScan operation
OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, NodeScanCtx n,
		RSIndex *idx, FT_FilterNode *filter);


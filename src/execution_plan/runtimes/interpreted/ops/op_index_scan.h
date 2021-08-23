/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../../graph/graph.h"
#include "../../../../index/index.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/shared/scan_functions.h"

typedef struct {
	RT_OpBase op;
	Graph *g;
	bool rebuild_index_query;           // should we rebuild RediSearch index query for each input record
	RSIndex *idx;                       // index to query
	NodeScanCtx n;                      // label data of node being scanned
	uint nodeRecIdx;                    // index of the node being scanned in the Record
	RSResultsIterator *iter;            // rediSearch iterator over an index with the appropriate filters
	FT_FilterNode *filter;              // filter from which to compose index query
	FT_FilterNode *unresolved_filters;  // subset of filter, contains filters that couldn't be resolved by index
	Record child_record;                // the Record this op acts on if it is not a tap
} RT_IndexScan;

// creates a new IndexScan operation
RT_OpBase *RT_NewIndexScanOp(const RT_ExecutionPlan *plan, Graph *g, NodeScanCtx n,
		RSIndex *idx, FT_FilterNode *filter);

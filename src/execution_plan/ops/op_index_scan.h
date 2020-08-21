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
	NodeScanCtx n;           /* Label data of node being scanned. */
	uint nodeRecIdx;            /* Index of the node being scanned in the Record. */
	RSQNode *rs_query_node;     /* RediSearch query node used to construct iterator. */
	RSResultsIterator *iter;    /* RediSearch iterator over an index with the appropriate filters. */
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} IndexScan;

/* Creates a new IndexScan operation */
OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, NodeScanCtx n, RSIndex *idx,
					   RSQNode *rs_query_node);


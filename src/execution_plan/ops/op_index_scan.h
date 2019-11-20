/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../index/index.h"
#include "redisearch_api.h"

typedef struct {
	OpBase op;
	Graph *g;
	RSIndex *idx;
	const QGNode *n;
	uint nodeRecIdx;
	RSResultsIterator *iter;
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} IndexScan;

/* Creates a new IndexScan operation */
OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, const QGNode *n, RSIndex *idx,
					   RSResultsIterator *iter);


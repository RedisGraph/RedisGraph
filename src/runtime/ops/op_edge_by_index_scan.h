/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../IR/execution_plan/execution_plan.h"
#include "../../storage/graph/graph.h"
#include "redisearch_api.h"

typedef struct {
	OpBase op;
	Graph *g;
	bool rebuild_index_query;           // should we rebuild RediSearch index query for each input record
	RSIndex *idx;                       // index to query
	QGEdge *edge;                       // edge scanned
	int edgeRecIdx;                     // record index of source node
	int srcRecIdx;                      // record index of destination node
	int destRecIdx;                     // record index of edge
	bool srcAware;                      // src node already resolved
	bool destAware;                     // dest node already resolved
	RSResultsIterator *iter;            // iterator over an index
	FT_FilterNode *filter;              // index query
	AR_ExpNode *current_src_node_id;    // current source node id
	AR_ExpNode *current_dest_node_id;   // current destination node id
	FT_FilterNode *unresolved_filters;  // subset of filter, contains filters that couldn't be resolved by index
	Record child_record;                // input record in case op ins't a tap
} OpEdgeIndexScan;

// creates a new OpEdgeIndexScan operation
OpBase *NewEdgeIndexScanOp
(
	const ExecutionPlan *plan,
	Graph *g,
	QGEdge *e,
	RSIndex *idx,
	FT_FilterNode *filter
);


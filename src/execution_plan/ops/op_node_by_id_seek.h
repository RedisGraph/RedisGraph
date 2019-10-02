/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"

#define ID_RANGE_UNBOUND -1

/* Node by ID seek locates an entity by its ID */
typedef struct {
	OpBase op;
	Graph *g;               // Graph object.
	NodeID currentId;       // Current ID fetched.
	NodeID minId;           // Min ID to fetch.
	NodeID maxId;           // Max ID to fetch.
	bool minInclusive;      // Include min ID.
	bool maxInclusive;      // Include max ID.
	int nodeRecIdx;         // Position of entity within record.
} OpNodeByIdSeek;

OpBase *NewNodeByIdSeekOp(const ExecutionPlan *plan, const QGNode *node, NodeID minId, NodeID maxId,
						  bool includeMin, bool includeMax);


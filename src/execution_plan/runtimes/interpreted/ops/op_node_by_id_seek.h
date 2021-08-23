/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../../graph/graph.h"
#include "../runtime_execution_plan.h"
#include "../../../../util/range/unsigned_range.h"

#define ID_RANGE_UNBOUND -1

/* Node by ID seek locates an entity by its ID */
typedef struct {
	RT_OpBase op;
	Graph *g;               // Graph object.
	Record child_record;    // The Record this op acts on if it is not a tap.
	const char *alias;      // Alias of the node being scanned by this op.
	NodeID currentId;       // Current ID fetched.
	NodeID minId;           // Min ID to fetch.
	NodeID maxId;           // Max ID to fetch.
	int nodeRecIdx;         // Position of entity within record.
} RT_NodeByIdSeek;

RT_OpBase *RT_NewNodeByIdSeekOp(const RT_ExecutionPlan *plan, const char *alias, UnsignedRange *id_range);

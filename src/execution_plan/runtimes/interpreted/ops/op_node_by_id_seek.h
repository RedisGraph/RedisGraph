/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../../graph/graph.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_node_by_id_seek.h"
#include "../../../../util/range/unsigned_range.h"

#define ID_RANGE_UNBOUND -1

/* Node by ID seek locates an entity by its ID */
typedef struct {
	RT_OpBase op;
	const NodeByIdSeek *op_desc;
	Graph *g;               // Graph object.
	Record child_record;    // The Record this op acts on if it is not a tap.
	NodeID currentId;       // Current ID fetched.
	NodeID maxId;           // Max ID to fetch.
	uint nodeRecIdx;        // Position of entity within record.
} RT_NodeByIdSeek;

RT_OpBase *RT_NewNodeByIdSeekOp(const RT_ExecutionPlan *plan, const NodeByIdSeek *op_desc);

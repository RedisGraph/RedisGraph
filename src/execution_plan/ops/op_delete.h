/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/entities/node.h"
#include "../../resultset/resultset_statistics.h"
#include "rax.h"
/* Deletes entities specified within the DELETE clause. */


typedef struct {
	OpBase op;
	GraphContext *gc;
	uint node_count;
	uint edge_count;
	uint *nodes_to_delete;
	uint *edges_to_delete;
	Node *deleted_nodes;    // Array of nodes to be removed.
	Edge *deleted_edges;    // Array of edges to be removed.

	ResultSetStatistics *stats;
} OpDelete;

OpBase *NewDeleteOp(const ExecutionPlan *plan, uint *nodes_ref, uint *edges_ref, ResultSetStatistics *stats);

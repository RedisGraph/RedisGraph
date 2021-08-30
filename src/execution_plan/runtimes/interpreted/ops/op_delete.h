/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "rax.h"
#include "../../../ops/op_delete.h"
#include "../runtime_execution_plan.h"
#include "../../../../graph/entities/node.h"
#include "../../../../resultset/resultset_statistics.h"

// Deletes entities specified within the DELETE clause
typedef struct {
	RT_OpBase op;
	const OpDelete *op_desc;
	GraphContext *gc;
	Node *deleted_nodes;    // Array of nodes to be removed.
	Edge *deleted_edges;    // Array of edges to be removed.

	ResultSetStatistics *stats;
} RT_OpDelete;

RT_OpBase *RT_NewDeleteOp(const RT_ExecutionPlan *plan, const OpDelete *op_desc);

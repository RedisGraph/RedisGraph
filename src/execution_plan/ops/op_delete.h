/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
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
	AR_ExpNode **exps;      // Expressions evaluated to an entity about to be deleted.
	uint exp_count;         // Number of expressions.
	Node *deleted_nodes;    // Array of nodes to be removed.
	Edge *deleted_edges;    // Array of edges to be removed.

	ResultSetStatistics *stats;
} OpDelete;

OpBase *NewDeleteOp(const ExecutionPlan *plan, AR_ExpNode **exps);

void DeleteEntities(OpDelete *op, bool is_rollback);

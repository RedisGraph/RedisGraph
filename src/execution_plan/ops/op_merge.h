/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../ast/ast_shared.h"
#include "../../resultset/resultset_statistics.h"

/* Merge execution plan operation,
 * this operation will create a pattern P if it doesn't not exists
 * in the graph, the pattern must be either fully matched, in which case
 * we'll simply return, otherwise (no match/ partial match) the ENTIRE pattern
 * is created. */

typedef struct {
	OpBase op;                        // Base op.
	GraphContext *gc;                 // Graph data.
	bool matched;                     // Has the entire pattern been matched?
	bool created;                     // Has the entire pattern been created?
	ResultSetStatistics *stats;       // Required for statistics updates.
	NodeCreateCtx *nodes_to_merge;
	EdgeCreateCtx *edges_to_merge;
} OpMerge;

OpBase *NewMergeOp(const ExecutionPlan *plan, ResultSetStatistics *stats,
				   NodeCreateCtx *nodes_to_merge, EdgeCreateCtx *edges_to_merge);

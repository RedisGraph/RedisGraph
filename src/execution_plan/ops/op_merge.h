/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../execution_plan.h"
#include "../../resultset/resultset_statistics.h"

/* Merge execution plan operation,
 * this operation will create a pattern P if it doesn't not exists
 * in the graph, the pattern must be either fully matched, in which case
 * we'll simply return, otherwise (no match/ partial match) the ENTIRE pattern
 * is created. */

typedef struct {
	OpBase op;                        // Base op.
	// bool should_create_pattern;       // The pattern has not been evaluated successfully, should be created.
	OpBase *bound_variable_stream;
	OpBase *match_stream;
	Argument *match_argument_tap;
	OpBase *create_stream;
	Argument *create_argument_tap;
	Record *records;
	ResultSetStatistics *stats;       // Required for statistics updates. (might want for ON MATCH SET)
} OpMerge;

OpBase *NewMergeOp(const ExecutionPlan *plan, ResultSetStatistics *stats);

// Fix the order of a Merge op's child streams if they have been shuffled by optimizations.
void Merge_SetStreams(OpBase *op);


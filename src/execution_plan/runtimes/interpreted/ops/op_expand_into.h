/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../../graph/graph.h"
#include "../../../../graph/entities/edge.h"
#include "../../../ops/shared/traverse_functions.h"
#include "../../../../arithmetic/algebraic_expression.h"

typedef struct {
	RT_OpBase op;
	Graph *graph;
	AlgebraicExpression *ae;
	RG_Matrix F;                // Filter matrix.
	RG_Matrix M;                // Algebraic expression result.
	EdgeTraverseCtx *edge_ctx;  // Edge collection data if the edge needs to be set.
	uint srcNodeIdx;            // Source node index into record.
	uint destNodeIdx;           // Destination node index into record.
	uint record_count;          // Number of held records.
	uint record_cap;            // Max number of records to process.
	Record *records;            // Array of records.
	Record r;                   // Currently selected record.
} RT_OpExpandInto;

RT_OpBase *RT_NewExpandIntoOp(const RT_ExecutionPlan *plan, AlgebraicExpression *ae);

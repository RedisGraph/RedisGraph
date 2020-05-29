/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "shared/traverse_functions.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../graph/entities/edge.h"
#include "../../arithmetic/algebraic_expression.h"

typedef struct {
	OpBase op;
	Graph *graph;
	AlgebraicExpression *ae;
	GrB_Matrix F;               // Filter matrix.
	GrB_Matrix M;               // Algebraic expression result.
	EdgeTraverseCtx *edge_ctx;  // Edge collection data if the edge needs to be set.
	int srcNodeIdx;             // Source node index into record.
	int destNodeIdx;            // Destination node index into record.
	uint recordCount;           // Number of held records.
	uint recordsCap;            // Max number of records to process.
	Record *records;            // Array of records.
	Record r;                   // Currently selected record.
} OpExpandInto;

OpBase *NewExpandIntoOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae);


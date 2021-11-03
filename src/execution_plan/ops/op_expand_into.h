/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
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
	RG_Matrix F;                // filter matrix
	RG_Matrix M;                // algebraic expression result
	EdgeTraverseCtx *edge_ctx;  // edge collection data if the edge needs to be set
	int srcNodeIdx;             // source node index into record
	int destNodeIdx;            // destination node index into record
	bool single_operand;        // expression contains a single operand
	uint record_count;          // number of held records
	uint record_cap;            // max number of records to process
	Record *records;            // array of records
	Record r;                   // currently selected record
} OpExpandInto;

OpBase *NewExpandIntoOp
(
	const ExecutionPlan *plan,
	Graph *g,
	AlgebraicExpression *ae
);


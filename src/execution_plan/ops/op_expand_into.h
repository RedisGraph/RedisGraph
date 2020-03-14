/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../graph/entities/edge.h"
#include "../../arithmetic/algebraic_expression.h"

#define TRAVERSE_RECORDS_CAP 16

typedef struct {
	OpBase op;
	Graph *graph;
	AlgebraicExpression *ae;
	GrB_Matrix F;               // Filter matrix.
	GrB_Matrix M;               // Algebraic expression result.
	int *edgeRelationTypes;     // One or more relation types.
	int edgeRelationCount;      // length of edgeRelationTypes.
	Edge *edges;                // Discovered edges.
	bool setEdge;               // Edge needs to be set.
	GxB_MatrixTupleIter *iter;  // Iterator over M.
	int srcNodeIdx;             // Index into record.
	int destNodeIdx;            // Index into record.
	int edgeIdx;                // Index into record.
	AR_ExpNode *recordsCapExpr; // Expression to initialize records cap
	uint recordsCap;            // Max number of records to process.
	uint recordCount;           // Number of records to process.
	Record *records;            // Array of records.
	Record r;                   // Current selected record.
} OpExpandInto;

OpBase *NewExpandIntoOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae,
						AR_ExpNode *records_cap_expr);


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

typedef struct {
	OpBase op;
	Graph *graph;
	AlgebraicExpression *ae;
	GrB_Matrix F;               // Filter matrix.
	GrB_Matrix M;               // Algebraic expression result.
	int *edgeRelationTypes;     // One or more relation types.
	int edgeRelationCount;      // length of edgeRelationTypes.
	bool setEdge;               // Edge needs to be set.
	Edge *edges;                // Discovered edges.
	int srcNodeIdx;             // Index into record.
	int destNodeIdx;            // Index into record.
	int edgeIdx;                // Index into record.
	uint recordsCap;            // Max number of records to process.
	uint recordCount;           // Number of records to process.
	GRAPH_EDGE_DIR direction;   // The direction of the referenced edge being traversed.
	Record *records;            // Array of records.
	Record r;                   // Current selected record.
} OpExpandInto;

OpBase *NewExpandIntoOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae);


/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* OP Traverse */
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
	GxB_MatrixTupleIter *iter;  // Iterator over M.
	int srcNodeIdx;             // Index into record.
	int destNodeIdx;            // Index into record.
	int edgeIdx;                // Index into record.
	int recordsCap;             // Max number of records to process.
	int recordCount;            // Number of records to process.
	GRAPH_EDGE_DIR direction;   // The direction of the referenced edge being traversed.
	Record *records;            // Array of records.
	Record r;                   // Current selected record.
} CondTraverse;

/* Creates a new Traverse operation */
OpBase *NewCondTraverseOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae);


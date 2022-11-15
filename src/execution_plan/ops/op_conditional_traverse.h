/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "shared/traverse_functions.h"
#include "../../graph/rg_matrix/rg_matrix_iter.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* OP Traverse */
typedef struct {
	OpBase op;
	Graph *graph;
	AlgebraicExpression *ae;
	RG_Matrix F;                // Filter matrix.
	RG_Matrix M;                // Algebraic expression result.
	EdgeTraverseCtx *edge_ctx;  // Edge collection data if the edge needs to be set.
	RG_MatrixTupleIter iter;    // Iterator over M.
	int srcNodeIdx;             // Source node index into record.
	int destNodeIdx;            // Destination node index into record.
	uint record_count;          // Number of held records.
	uint record_cap;            // Max number of records to process.
	Record *records;            // Array of records.
	Record r;                   // Currently selected record.
} OpCondTraverse;

/* Creates a new Traverse operation */
OpBase *NewCondTraverseOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae);


/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "./rg_matrix.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

// TuplesIter maintains information required
// to iterate over a RG_Matrix
typedef struct
{
    RG_Matrix rg_m;
    GxB_MatrixTupleIter* m_iter;     // internal m iterator
    GxB_MatrixTupleIter* dp_iter;    // internal delta plus iterator
} RG_MatrixTupleIter ;

// Create a new iterator
GrB_Info RG_MatrixTupleIter_new
(
	RG_MatrixTupleIter **iter,     // iterator to create
	const RG_Matrix A              // matrix to iterate over
);

GrB_Info RG_MatrixTupleIter_iterate_row
(
	RG_MatrixTupleIter *iter,      // iterator to use
	GrB_Index rowIdx               // row to iterate
);

GrB_Info RG_MatrixTupleIter_jump_to_row
(
	RG_MatrixTupleIter *iter,      // iterator to use
	GrB_Index rowIdx               // row to jump to
);

GrB_Info RG_MatrixTupleIter_iterate_range
(
	RG_MatrixTupleIter *iter,   // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
);

// Advance iterator
GrB_Info RG_MatrixTupleIter_next
(
	RG_MatrixTupleIter *iter,       // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	void *val,                      // optional value at A[row, col]
	bool *depleted                  // indicate if iterator depleted
);

// Reset iterator
GrB_Info RG_MatrixTupleIter_reset
(
	RG_MatrixTupleIter *iter       // iterator to reset
);

// Free iterator
GrB_Info RG_MatrixTupleIter_free
(
	RG_MatrixTupleIter *iter       // iterator to free
);


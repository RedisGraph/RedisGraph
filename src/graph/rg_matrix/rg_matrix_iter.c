/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "./rg_matrix_iter.h"
#include "../../util/rmalloc.h"

// create a new iterator
GrB_Info RG_MatrixTupleIter_new
(
	RG_MatrixTupleIter **iter,     // iterator to create
	const RG_Matrix A              // matrix to iterate over
) {
	GrB_Info info ;

	ASSERT(A != NULL) ;
	ASSERT(iter != NULL) ;

	GrB_Matrix M  = RG_MATRIX_M(A) ;
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A) ;

	RG_MatrixTupleIter *it = rm_calloc(1, sizeof(RG_MatrixTupleIter)) ;
	it->rg_m = A ;

	info = GxB_MatrixTupleIter_new(&(it->m_iter), M) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_new(&(it->dp_iter), DP) ;
	ASSERT(info == GrB_SUCCESS) ;

	*iter = it ;
	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_iterate_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info ;

	ASSERT(iter          != NULL) ;
	ASSERT(iter->m_iter  != NULL) ;
	ASSERT(iter->dp_iter != NULL) ;

	info = GxB_MatrixTupleIter_iterate_row(iter->m_iter, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_iterate_row(iter->dp_iter, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_jump_to_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info ;

	ASSERT(iter          != NULL) ;
	ASSERT(iter->m_iter  != NULL) ;
	ASSERT(iter->dp_iter != NULL) ;

	info = GxB_MatrixTupleIter_jump_to_row(iter->m_iter, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_jump_to_row(iter->dp_iter, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_iterate_range
(
	RG_MatrixTupleIter *iter,   // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	GrB_Info info = GrB_SUCCESS ;

	ASSERT(iter          != NULL) ;
	ASSERT(iter->m_iter  != NULL) ;
	ASSERT(iter->dp_iter != NULL) ;

	info = GxB_MatrixTupleIter_iterate_range(iter->m_iter, startRowIdx, endRowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_iterate_range(iter->dp_iter, startRowIdx, endRowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

// iterate over M matrix
static void _next_m_iter
(
	GxB_MatrixTupleIter *it, // iterator scanning M
	const GrB_Matrix DM,     // delta-minus, masking entries
	GrB_Index *row,          // optional extracted row index
	GrB_Index *col,          // optional extracted column index
	void *val,               // optional extracted value
	bool *depleted           // [output] true if iterator depleted
) {
	ASSERT(it       != NULL) ;
	ASSERT(DM       != NULL) ;
	ASSERT(depleted != NULL) ;

	GrB_Index  _row ;
	GrB_Index  _col ;
	GrB_Info   info ;

	do {
		info = GxB_MatrixTupleIter_next(it, &_row, &_col, val, depleted) ;
		ASSERT(info == GrB_SUCCESS) ;

		// iterator depleted, return
		if(*depleted) return ;
	} while (true) ;

	if(row) *row = _row ;
	if(col) *col = _col ;
}

// Advance iterator
GrB_Info RG_MatrixTupleIter_next
(
	RG_MatrixTupleIter *iter,       // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	void *val,                      // optional value at A[row, col]
	bool *depleted                  // indicate if iterator depleted
) {
	GrB_Info info ;

	ASSERT(iter          != NULL) ;
	ASSERT(depleted      != NULL) ;
	ASSERT(iter->m_iter  != NULL) ;
	ASSERT(iter->dp_iter != NULL) ;

	GrB_Matrix           DM       =  RG_MATRIX_DELTA_MINUS(iter->rg_m) ;
	GxB_MatrixTupleIter  *m_it    =  iter->m_iter                      ;
	GxB_MatrixTupleIter  *dp_it   =  iter->dp_iter                     ;

	_next_m_iter(m_it, DM, row, col, val, depleted) ;
	if(!depleted) return ;

	// than finish with the delta plus matrix
	return GxB_MatrixTupleIter_next(dp_it, row, col, val, depleted);
}

// Reset iterator, assumes the iterator is valid
GrB_Info RG_MatrixTupleIter_reset
(
	RG_MatrixTupleIter *iter       // iterator to reset
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	if(iter->m_iter == NULL || iter->dp_iter == NULL) return GrB_INVALID_VALUE; // no iterators to iterate over is invalid scenario

	info = GxB_MatrixTupleIter_reset(iter->m_iter);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_reset(iter->dp_iter);
	ASSERT(info == GrB_SUCCESS);

	return (GrB_SUCCESS);
}

// Update iterator to scan given matrix
GrB_Info RG_MatrixTupleIter_reuse
(
	RG_MatrixTupleIter *iter,       // iterator to update
	GrB_Matrix A                    // matrix to scan
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	ASSERT(A != NULL);

	info = GxB_MatrixTupleIter_free(iter->m_iter);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_free(iter->dp_iter);
	ASSERT(info == GrB_SUCCESS);

	return RG_MatrixTupleIter_new(&iter, A);
}

// Free iterator, assumes the iterator is valid
GrB_Info RG_MatrixTupleIter_free
(
	RG_MatrixTupleIter *iter       // iterator to free
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);

	info = GxB_MatrixTupleIter_free(iter->m_iter);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_free(iter->dp_iter);
	ASSERT(info == GrB_SUCCESS);

	rm_free(iter);

	return (GrB_SUCCESS);
}

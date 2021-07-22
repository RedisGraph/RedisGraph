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

	ASSERT(A != NULL) ;
	ASSERT(iter != NULL) ;

	GrB_Info info ;
	GrB_Matrix M  = RG_MATRIX_M(A) ;
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A) ;

	RG_MatrixTupleIter *it = rm_calloc(1, sizeof(RG_MatrixTupleIter)) ;
	it->A = A ;

	info = GxB_MatrixTupleIter_new(&(it->m_it), M) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_new(&(it->dp_it), DP) ;
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

	ASSERT(iter        != NULL) ;
	ASSERT(iter->m_it  != NULL) ;
	ASSERT(iter->dp_it != NULL) ;

	info = GxB_MatrixTupleIter_iterate_row(iter->m_it, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_iterate_row(iter->dp_it, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_jump_to_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info ;

	ASSERT(iter        != NULL) ;
	ASSERT(iter->m_it  != NULL) ;
	ASSERT(iter->dp_it != NULL) ;

	info = GxB_MatrixTupleIter_jump_to_row(iter->m_it, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_jump_to_row(iter->dp_it, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_iterate_range
(
	RG_MatrixTupleIter *iter,   // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	GrB_Info info ;

	ASSERT(iter        != NULL) ;
	ASSERT(iter->m_it  != NULL) ;
	ASSERT(iter->dp_it != NULL) ;

	info = GxB_MatrixTupleIter_iterate_range(iter->m_it, startRowIdx, endRowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_iterate_range(iter->dp_it, startRowIdx, endRowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

// iterate over M matrix
static GrB_Info _next_m_iter
(
	GxB_MatrixTupleIter *it, // iterator scanning M
	const GrB_Matrix DM,     // delta-minus, masked entries
	GrB_Index *row,          // optional extracted row index
	GrB_Index *col,          // optional extracted column index
	void *val,               // optional extracted value
	bool *depleted           // [output] true if iterator depleted
) {
	ASSERT(it       != NULL) ;
	ASSERT(DM       != NULL) ;
	ASSERT(depleted != NULL) ;

	GrB_Info   info ;
	GrB_Index  _row ;
	GrB_Index  _col ;

	do {
		info = GxB_MatrixTupleIter_next(it, &_row, &_col, val, depleted) ;
		ASSERT(info == GrB_SUCCESS) ;

		// iterator depleted, return
		if(*depleted) return info;

		bool x ;
 		GrB_Info delete_info = GrB_Matrix_extractElement_BOOL(&x, DM, _row, _col) ;
 		if(delete_info == GrB_NO_VALUE) break ; // entry isn't deleted, return
	} while (true) ;

	if(row) *row = _row ;
	if(col) *col = _col ;

	return info;
}

// advance iterator
GrB_Info RG_MatrixTupleIter_next
(
	RG_MatrixTupleIter *iter,       // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	void *val,                      // optional value at A[row, col]
	bool *depleted                  // [output] true if iterator depleted
) {
	ASSERT(iter        != NULL) ;
	ASSERT(depleted    != NULL) ;
	ASSERT(iter->m_it  != NULL) ;
	ASSERT(iter->dp_it != NULL) ;

	GrB_Matrix           DM       =  RG_MATRIX_DELTA_MINUS(iter->A) ;
	GxB_MatrixTupleIter  *m_it    =  iter->m_it                     ;
	GxB_MatrixTupleIter  *dp_it   =  iter->dp_it                    ;

	GrB_Info info = _next_m_iter(m_it, DM, row, col, val, depleted) ;
	if(!(*depleted)) return info ;

	// M iterator depleted, try delta-plus iterator
	return GxB_MatrixTupleIter_next(dp_it, row, col, val, depleted) ;
}

// reset iterator, assumes the iterator is valid
GrB_Info RG_MatrixTupleIter_reset
(
	RG_MatrixTupleIter *iter       // iterator to reset
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter        != NULL) ;
	ASSERT(iter->m_it  != NULL) ;
	ASSERT(iter->dp_it != NULL) ;

	info = GxB_MatrixTupleIter_reset(iter->m_it) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_reset(iter->dp_it) ;
	ASSERT(info == GrB_SUCCESS) ;

	return (GrB_SUCCESS) ;
}

// update iterator to scan given matrix
GrB_Info RG_MatrixTupleIter_reuse
(
	RG_MatrixTupleIter *iter,       // iterator to update
	const RG_Matrix A               // matrix to scan
) {
	ASSERT(A    != NULL) ;
	ASSERT(iter != NULL) ;

	GrB_Info info ;
	GrB_Matrix M  = RG_MATRIX_M(A) ;
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A) ;

	iter->A = A ;

	info = GxB_MatrixTupleIter_reuse(iter->m_it, M) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_reuse(iter->dp_it, DP) ;
	ASSERT(info == GrB_SUCCESS) ;

	return info;
}

// free iterator
GrB_Info RG_MatrixTupleIter_free
(
	RG_MatrixTupleIter **iter       // iterator to free
) {
	ASSERT(*iter != NULL);

	GrB_Info info ;

	info = GxB_MatrixTupleIter_free((*iter)->m_it) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_MatrixTupleIter_free((*iter)->dp_it) ;
	ASSERT(info == GrB_SUCCESS);

	rm_free(*iter);
	*iter = NULL;

	return (GrB_SUCCESS);
}


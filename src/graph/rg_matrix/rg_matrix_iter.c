/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
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

	info = GxB_Iterator_new(&it->m_it) ;
	ASSERT(info == GrB_SUCCESS) ;
	info = GxB_rowIterator_attach(it->m_it, M, NULL) ;
	ASSERT(info == GrB_SUCCESS) ;
	info = GxB_rowIterator_seekRow (it->m_it, 0) ;
	if(info == GrB_NO_VALUE) it->m_depleted = true ;
	else ASSERT(info == GrB_SUCCESS) ;

	info = GxB_Iterator_new(&it->dp_it) ;
	ASSERT(info == GrB_SUCCESS) ;
	info = GxB_rowIterator_attach(it->dp_it, DP, NULL) ;
	ASSERT(info == GrB_SUCCESS) ;
	info = GxB_rowIterator_seekRow (it->dp_it, 0) ;
	if(info == GrB_NO_VALUE) it->m_depleted = true ;
	else ASSERT(info == GrB_SUCCESS) ;

	*iter = it ;
	return info ;
}

GrB_Info RG_MatrixTupleIter_iterate_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info ;

	if(iter == NULL) return GrB_NULL_POINTER ;

	info = GxB_rowIterator_seekRow(iter->m_it, rowIdx) ;
	if(info == GrB_NO_VALUE) iter->m_depleted = true ;
	else ASSERT(info == GrB_SUCCESS) ;

	info = GxB_rowIterator_seekRow(iter->dp_it, rowIdx) ;
	if(info == GrB_NO_VALUE) iter->dp_depleted = true ;
	else ASSERT(info == GrB_SUCCESS) ;

	return info ;
}

GrB_Info RG_MatrixTupleIter_jump_to_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info ;

	if(iter == NULL) return GrB_NULL_POINTER ;

	info = GxB_rowIterator_seekRow(iter->m_it, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_rowIterator_seekRow(iter->dp_it, rowIdx) ;
	ASSERT(info == GrB_SUCCESS) ;

	return info ;
}

GrB_Info RG_MatrixTupleIter_iterate_range
(
	RG_MatrixTupleIter *iter,   // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	ASSERT(false);
	GrB_Info info = GrB_SUCCESS;

	// if(iter == NULL) return GrB_NULL_POINTER ;

	// info = GxB_MatrixTupleIter_iterate_range(&(iter->m_it), startRowIdx,
	// 		endRowIdx) ;
	// ASSERT(info == GrB_SUCCESS) ;

	// info = GxB_MatrixTupleIter_iterate_range(&(iter->dp_it), startRowIdx,
	// 		endRowIdx) ;
	// ASSERT(info == GrB_SUCCESS) ;

	return info ;
}

// iterate over M matrix
static GrB_Info _next_m_iter
(
	GxB_Iterator it,         // iterator scanning M
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
		// iterator depleted, return
		if(*depleted) return info;

		_row = GxB_rowIterator_getRowIndex (it) ;
		_col = GxB_rowIterator_getColIndex (it) ;
		if(val) GxB_Iterator_get_UDT (it, val) ;
		info = GxB_rowIterator_nextCol (it) ;
		if (info != GrB_SUCCESS) {
			info = GxB_rowIterator_nextRow (it) ;
			*depleted = info == GxB_EXHAUSTED ;
		}

		bool x ;
 		GrB_Info delete_info = GrB_Matrix_extractElement_BOOL(&x, DM, _row, _col) ;
 		if(delete_info == GrB_NO_VALUE) break ; // entry isn't deleted, return
	} while (true) ;

	if(row) *row = _row ;
	if(col) *col = _col ;

	return info ;
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
	if(iter == NULL) return GrB_NULL_POINTER;

	ASSERT(iter        != NULL) ;
	ASSERT(depleted    != NULL) ;

	GrB_Info             info     =  GrB_SUCCESS                    ;
	GrB_Matrix           DM       =  RG_MATRIX_DELTA_MINUS(iter->A) ;
	GxB_Iterator         m_it     =  iter->m_it                     ;
	GxB_Iterator         dp_it    =  iter->dp_it                    ;

	if(!iter->m_depleted) {
		info = _next_m_iter(m_it, DM, row, col, val, depleted) ;
		if(!(*depleted)) {
			return info ;
		} else {
			iter->m_depleted = true;
			return info;
		}
	}

	if(iter->dp_depleted) {
		*depleted = true;
		return GrB_SUCCESS;
	}

	if(row) *row = GxB_rowIterator_getRowIndex (dp_it) ;
	if(col) *col = GxB_rowIterator_getColIndex (dp_it) ;
	if(val) GxB_Iterator_get_UDT (dp_it, val) ;
	info = GxB_rowIterator_nextCol (dp_it) ;
	if (info != GrB_SUCCESS) {
		info = GxB_rowIterator_nextRow (dp_it) ;
		iter->dp_depleted = info == GxB_EXHAUSTED ;
	}
	return GrB_SUCCESS ;
}

// reset iterator, assumes the iterator is valid
GrB_Info RG_MatrixTupleIter_reset
(
	RG_MatrixTupleIter *iter       // iterator to reset
) {
	GrB_Info info = GrB_SUCCESS;

	if(iter == NULL) return GrB_NULL_POINTER ;

	info = GxB_rowIterator_seekRow(iter->m_it, 0) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_rowIterator_seekRow(iter->dp_it, 0) ;
	ASSERT(info == GrB_SUCCESS) ;

	return info ;
}

// update iterator to scan given matrix
GrB_Info RG_MatrixTupleIter_reuse
(
	RG_MatrixTupleIter *iter,       // iterator to update
	const RG_Matrix A               // matrix to scan
) {
	if(A == NULL) return GrB_NULL_POINTER ;
	if(iter == NULL) return GrB_NULL_POINTER ;

	GrB_Info info ;
	GrB_Matrix M  = RG_MATRIX_M(A) ;
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A) ;

	iter->A = A ;

	info = GxB_rowIterator_attach(iter->m_it, M, NULL) ;
	info = GxB_rowIterator_seekRow(iter->m_it, 0) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_rowIterator_attach(iter->dp_it, DP, NULL) ;
	info = GxB_rowIterator_seekRow(iter->dp_it, 0) ;
	ASSERT(info == GrB_SUCCESS) ;

	return info;
}

// free iterator
GrB_Info RG_MatrixTupleIter_free
(
	RG_MatrixTupleIter **iter       // iterator to free
) {
	ASSERT(*iter != NULL) ;

	rm_free(*iter) ;
	*iter = NULL ;

	return GrB_SUCCESS ;
}


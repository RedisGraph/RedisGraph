/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "./rg_matrix_iter.h"
#include "../../util/rmalloc.h"

static inline void _set_iter_range
(
	GxB_Iterator it,
	GrB_Index min_row,
	GrB_Index max_row,
	bool *depleted
) {
	GrB_Info info = GxB_rowIterator_seekRow (it, min_row) ;

	// exhausted -> depleted
	if (info == GxB_EXHAUSTED) {
		*depleted = true ;
		return ;
	}

	// make sure seekRow didn't over-reached
	if (GxB_rowIterator_getRowIndex (it) > max_row) {
		*depleted = true ;
		return ;
	}

	// iterator is within min/max range bounds
	if (info == GrB_SUCCESS) {
		*depleted = false ;
		return ;
	}

	// seek to first none empty row
	while (info == GrB_NO_VALUE && GxB_rowIterator_getRowIndex(it) < max_row) {
		info = GxB_rowIterator_nextRow (it) ;
	}

	*depleted = (info != GrB_SUCCESS ||
			     GxB_rowIterator_getRowIndex(it) > max_row) ;
}

static inline GxB_Iterator _init_iter
(
	GrB_Matrix m,
	GrB_Index min_row,
	GrB_Index max_row,
	bool *depleted
) {
	ASSERT(depleted != NULL) ;

	*depleted = true ; // default

	GxB_Iterator it ;
	GrB_Index nvals ;
	GrB_Info info ;
	UNUSED(info) ;

	info = GrB_Matrix_nvals(&nvals, m) ;
	ASSERT(info == GrB_SUCCESS) ;

	info = GxB_Iterator_new(&it) ;
	ASSERT(info == GrB_SUCCESS) ;

	if(nvals > 0) {
		info = GxB_rowIterator_attach(it, m, NULL) ;
		ASSERT(info == GrB_SUCCESS) ;
		_set_iter_range(it, min_row, max_row, depleted) ;
	}

	return it ;
}

// create a new iterator
GrB_Info RG_MatrixTupleIter_new
(
	RG_MatrixTupleIter **iter,     // iterator to create
	const RG_Matrix A              // matrix to iterate over
) {
	ASSERT(A != NULL) ;
	ASSERT(iter != NULL) ;

	GrB_Matrix M  = RG_MATRIX_M(A) ;
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A) ;

	RG_MatrixTupleIter *it = rm_calloc(1, sizeof(RG_MatrixTupleIter)) ;
	it->A = A ;
	it->min_row = 0 ;
	it->max_row = ULLONG_MAX ;

	it->m_it = _init_iter(M, it->min_row, it->max_row, &it->m_depleted) ;
	it->dp_it = _init_iter(DP, it->min_row, it->max_row, &it->dp_depleted) ;

	*iter = it ;
	return GrB_SUCCESS ;
}

GrB_Info RG_MatrixTupleIter_iterate_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	if(iter == NULL) return GrB_NULL_POINTER ;

	iter->min_row = rowIdx ;
	iter->max_row = rowIdx ;

	_set_iter_range(iter->m_it, iter->min_row, iter->max_row, &iter->m_depleted) ;
	_set_iter_range(iter->dp_it, iter->min_row, iter->max_row, &iter->dp_depleted) ;

	return GrB_SUCCESS ;
}

GrB_Info RG_MatrixTupleIter_jump_to_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	if(iter == NULL) return GrB_NULL_POINTER ;

	iter->min_row = rowIdx ;

	_set_iter_range(iter->m_it, iter->min_row, iter->max_row, &iter->m_depleted) ;
	_set_iter_range(iter->dp_it, iter->min_row, iter->max_row, &iter->dp_depleted) ;

	return GrB_SUCCESS ;
}

GrB_Info RG_MatrixTupleIter_iterate_range
(
	RG_MatrixTupleIter *iter,   // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	if(iter == NULL) return GrB_NULL_POINTER ;

	iter->min_row = startRowIdx ;
	iter->max_row = endRowIdx ;

	_set_iter_range(iter->m_it, iter->min_row, iter->max_row, &iter->m_depleted) ;
	_set_iter_range(iter->dp_it, iter->min_row, iter->max_row, &iter->dp_depleted) ;

	return GrB_SUCCESS ;
}

static void _iter_next
(
	GxB_Iterator it,
	GrB_Index max_row,
	bool *depleted
) {
	GrB_Info   info ;

	info = GxB_rowIterator_nextCol (it) ;
	if (info != GrB_SUCCESS) {
		info = GxB_rowIterator_nextRow (it) ;
		// in-case iterator maintains number of yield values, we can use nvals here
		// for a quick return!
		// TODO: see how much time does it take to figure out that there are no more none-empty rows
		// in a 50MX50M matrix with "last" entry at position [10M, 0] (seeking 40M rows)
		while(info == GrB_NO_VALUE && GxB_rowIterator_getRowIndex(it) < max_row) {
			info = GxB_rowIterator_nextRow (it) ;
		}

		// prep for next call to `_next_m_iter`
		*depleted = info != GrB_SUCCESS || GxB_rowIterator_getRowIndex(it) > max_row ;
	}
}

// iterate over M matrix
static GrB_Info _next_m_iter_bool
(
	RG_MatrixTupleIter *iter,  // iterator scanning M
	const GrB_Matrix DM,       // delta-minus, masked entries
	GrB_Index *row,            // optional extracted row index
	GrB_Index *col,            // optional extracted column index
	bool *val,                 // optional extracted value
	bool *depleted             // [output] true if iterator depleted
) {
	ASSERT(iter     != NULL) ;
	ASSERT(DM       != NULL) ;
	ASSERT(depleted != NULL) ;

	GrB_Index  _row ;
	GrB_Index  _col ;

	GxB_Iterator it = iter->m_it;

	do {
		// iterator depleted, return
		if(*depleted) return GrB_NO_VALUE;

		_row = GxB_rowIterator_getRowIndex (it) ;
		_col = GxB_rowIterator_getColIndex (it) ;
		// TODO: depending on the matrix type BOOL/UINT64 use the right typed function(s)
		if(val) *val = GxB_Iterator_get_BOOL (it) ;

		// prep value for next iteration
		_iter_next(it, iter->max_row, depleted);

		bool x ;
 		GrB_Info delete_info = GrB_Matrix_extractElement_BOOL(&x, DM, _row, _col) ;
 		if(delete_info == GrB_NO_VALUE) break ; // entry isn't deleted, return
	} while (true) ;

	if(row) *row = _row ;
	if(col) *col = _col ;

	return GrB_SUCCESS ;
}

// advance iterator
GrB_Info RG_MatrixTupleIter_next_BOOL
(
	RG_MatrixTupleIter *iter,       // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	bool *val                       // optional value at A[row, col]
) {
	if(iter == NULL) return GrB_NULL_POINTER ;

	GrB_Info             info     =  GrB_SUCCESS                    ;
	GrB_Matrix           DM       =  RG_MATRIX_DELTA_MINUS(iter->A) ;
	GxB_Iterator         dp_it    =  iter->dp_it                    ;

	if(!iter->m_depleted) {
		info = _next_m_iter_bool(iter, DM, row, col, val, &iter->m_depleted) ;
		if(info == GrB_SUCCESS) return GrB_SUCCESS ;
	}

	if(iter->dp_depleted) {
		return GxB_EXHAUSTED ;
	}

	if(row) *row = GxB_rowIterator_getRowIndex (dp_it) ;
	if(col) *col = GxB_rowIterator_getColIndex (dp_it) ;
	if(val) *val = GxB_Iterator_get_BOOL (dp_it) ;

	// prep value for next iteration
	_iter_next(dp_it, iter->max_row, &iter->dp_depleted);

	return GrB_SUCCESS ;
}

// iterate over M matrix
static GrB_Info _next_m_iter_uint64
(
	RG_MatrixTupleIter *iter,  // iterator scanning M
	const GrB_Matrix DM,       // delta-minus, masked entries
	GrB_Index *row,            // optional extracted row index
	GrB_Index *col,            // optional extracted column index
	uint64_t *val,             // optional extracted value
	bool *depleted             // [output] true if iterator depleted
) {
	ASSERT(iter     != NULL) ;
	ASSERT(DM       != NULL) ;
	ASSERT(depleted != NULL) ;

	GrB_Index  _row ;
	GrB_Index  _col ;

	GxB_Iterator it = iter->m_it;

	do {
		// iterator depleted, return
		if(*depleted) return GrB_NO_VALUE;

		_row = GxB_rowIterator_getRowIndex (it) ;
		_col = GxB_rowIterator_getColIndex (it) ;
		if(val) *val = GxB_Iterator_get_UINT64 (it) ;

		// prep value for next iteration
		_iter_next(it, iter->max_row, depleted);

		bool x ;
 		GrB_Info delete_info = GrB_Matrix_extractElement_BOOL(&x, DM, _row, _col) ;
 		if(delete_info == GrB_NO_VALUE) break ; // entry isn't deleted, return
	} while (true) ;

	if(row) *row = _row ;
	if(col) *col = _col ;

	return GrB_SUCCESS ;
}

// advance iterator
GrB_Info RG_MatrixTupleIter_next_UINT64
(
	RG_MatrixTupleIter *iter,       // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	uint64_t *val                  // optional value at A[row, col]
) {
	if(iter == NULL) return GrB_NULL_POINTER ;

	GrB_Info             info     =  GrB_SUCCESS                    ;
	GrB_Matrix           DM       =  RG_MATRIX_DELTA_MINUS(iter->A) ;
	GxB_Iterator         dp_it    =  iter->dp_it                    ;

	if(!iter->m_depleted) {
		info = _next_m_iter_uint64(iter, DM, row, col, val, &iter->m_depleted) ;
		if(info == GrB_SUCCESS) return GrB_SUCCESS ;
	}

	if(iter->dp_depleted) {
		return GxB_EXHAUSTED ;
	}

	if(row) *row = GxB_rowIterator_getRowIndex (dp_it) ;
	if(col) *col = GxB_rowIterator_getColIndex (dp_it) ;
	if(val) *val = GxB_Iterator_get_UINT64 (dp_it) ;

	// prep value for next iteration
	_iter_next(dp_it, iter->max_row, &iter->dp_depleted);

	return GrB_SUCCESS ;
}

// reset iterator, assumes the iterator is valid
GrB_Info RG_MatrixTupleIter_reset
(
	RG_MatrixTupleIter *iter       // iterator to reset
) {
	GrB_Info info = GrB_SUCCESS;

	if(iter == NULL) return GrB_NULL_POINTER ;

	_set_iter_range(iter->m_it, iter->min_row, iter->max_row, &iter->m_depleted) ;
	_set_iter_range(iter->dp_it, iter->min_row, iter->max_row, &iter->dp_depleted) ;

	return info ;
}

// update iterator to scan given matrix
GrB_Info RG_MatrixTupleIter_attach
(
	RG_MatrixTupleIter *iter,       // iterator to update
	const RG_Matrix A               // matrix to scan
) {
	if(A == NULL) return GrB_NULL_POINTER ;
	if(iter == NULL) return GrB_NULL_POINTER ;

	GrB_Matrix M  = RG_MATRIX_M(A) ;
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A) ;

	iter->A = A ;
	iter->min_row = 0 ;
	iter->max_row = ULLONG_MAX ;

	if(iter->m_it != NULL) GrB_free (&iter->m_it) ;
	if(iter->dp_it != NULL) GrB_free (&iter->dp_it) ;

	iter->m_it = _init_iter(M, iter->min_row, iter->max_row, &iter->m_depleted) ;
	iter->dp_it = _init_iter(DP, iter->min_row, iter->max_row, &iter->dp_depleted) ;

	return GrB_SUCCESS ;
}

// free iterator data
GrB_Info RG_MatrixTupleIter_free_internals
(
	RG_MatrixTupleIter *iter       // iterator to free
) {
	ASSERT(iter != NULL) ;

	if(iter->m_it  != NULL) GrB_free (&iter->m_it) ;
	if(iter->dp_it != NULL) GrB_free (&iter->dp_it) ;

	iter->m_it  = NULL ;
	iter->dp_it = NULL ;

	return GrB_SUCCESS ;
}

// free iterator
GrB_Info RG_MatrixTupleIter_free
(
	RG_MatrixTupleIter **iter       // iterator to free
) {
	ASSERT(*iter != NULL) ;

	RG_MatrixTupleIter_free_internals(*iter);

	rm_free(*iter) ;
	*iter = NULL ;

	return GrB_SUCCESS ;
}

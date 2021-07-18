/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "./rg_matrix_iter.h"
#include "../../util/rmalloc.h"s

// Create a new iterator
GrB_Info RG_MatrixTupleIter_new
(
	RG_MatrixTupleIter **iter,     // iterator to create
	const RG_Matrix A              // matrix to iterate over
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(A != NULL);
	ASSERT(iter != NULL);

	int sparsity_type;
	GxB_Matrix_Option_get(A, GxB_SPARSITY_CONTROL, &sparsity_type);
	ASSERT(sparsity_type == GxB_SPARSE || sparsity_type == GxB_HYPERSPARSE);

	GrB_Matrix M  = RG_MATRIX_M(A);
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A);

	RG_MatrixTupleIter *it = rm_calloc(1, sizeof(RG_MatrixTupleIter));
	*iter = it;
	info = GxB_MatrixTupleIter_new(&(*iter)->m_iter, M);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_new(&(*iter)->dp_iter, DP);
	ASSERT(info == GrB_SUCCESS);

	return info ;
}

GrB_Info RG_MatrixTupleIter_iterate_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	if(iter->m_iter == NULL || iter->dp_iter == NULL) return GrB_INVALID_VALUE; // no iterators to iterate over is invalid scenario

	info = GxB_MatrixTupleIter_iterate_row(iter->m_iter, rowIdx);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_iterate_row(iter->dp_iter, rowIdx);
	ASSERT(info == GrB_SUCCESS);

	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_jump_to_row
(
	RG_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	if(iter->m_iter == NULL || iter->dp_iter == NULL) return GrB_INVALID_VALUE; // no iterators to iterate over is invalid scenario

	info = GxB_MatrixTupleIter_jump_to_row(iter->m_iter, rowIdx);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_jump_to_row(iter->dp_iter, rowIdx);
	ASSERT(info == GrB_SUCCESS);

	return (GrB_SUCCESS) ;
}

GrB_Info RG_MatrixTupleIter_iterate_range
(
	RG_MatrixTupleIter *iter,   // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	if(iter->m_iter == NULL || iter->dp_iter == NULL) return GrB_INVALID_VALUE; // no iterators to iterate over is invalid scenario

	info = GxB_MatrixTupleIter_iterate_range(iter->m_iter, startRowIdx, endRowIdx);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_iterate_range(iter->dp_iter, startRowIdx, endRowIdx);
	ASSERT(info == GrB_SUCCESS);

	return (GrB_SUCCESS);
}

// Advance iterator
GrB_Info RG_MatrixTupleIter_next
(
	RG_MatrixTupleIter *iter,       // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	bool *depleted                  // indicate if iterator depleted
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	ASSERT(depleted != NULL);
	if(iter->m_iter == NULL || iter->dp_iter == NULL) return GrB_INVALID_VALUE; // no iterators to iterate over is invalid scenario

	GrB_Matrix DM = RG_MATRIX_DELTA_MINUS(iter->rg_m);
	GxB_MatrixTupleIter *iter_cur = iter->m_iter;
	GrB_Index nnz_idx = iter_cur->nnz_idx;

	// first finish with the M matrix
	if(nnz_idx < iter_cur->nvals) {
		// move next till we found not deleted value
		do
		{		
			info = GxB_MatrixTupleIter_next(iter_cur, row, col, depleted);
			ASSERT(info == GrB_SUCCESS);

			if(*depleted) {
				break;
			}
			info = GrB_Matrix_extractElement(NULL, DM, *row, *col);
		} while (info == GrB_SUCCESS);
		if(!*depleted){
			return (GrB_SUCCESS);
		}
	}

	// than finish with the delta plus matrix
	iter_cur = iter->dp_iter;

	return GxB_MatrixTupleIter_next(iter_cur, row, col, depleted);
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

	return RG_MatrixTupleIter_init(&iter, A);
}

// Free iterator, assumes the iterator is valid
GrB_Info RG_MatrixTupleIter_free
(
	RG_MatrixTupleIter *iter       // iterator to free
) {
	GrB_Info info = GrB_SUCCESS;

	ASSERT(iter != NULL);
	ASSERT(A != NULL);

	info = GxB_MatrixTupleIter_free(iter->m_iter);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_MatrixTupleIter_free(iter->dp_iter);
	ASSERT(info == GrB_SUCCESS);

	rm_free(iter);

	return (GrB_SUCCESS);
}
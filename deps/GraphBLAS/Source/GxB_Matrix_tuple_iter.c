//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// Sets iterator as depleted.
static inline void _EmptyIterator
(
	GxB_MatrixTupleIter *iter   // Iterator to deplete.
) {
	iter->nvals = 0;
	iter->nnz_idx = 0;
}

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter,     // iterator to create
	GrB_Matrix A                    // matrix to iterate over
) {
	GB_WHERE(A, "GxB_MatrixTupleIter_new (A)") ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	// make sure matrix is not bitmap or full
	GxB_set(A, GxB_SPARSITY_CONTROL, GxB_SPARSE) ;

	size_t     size   ;
	GrB_Type   type   ;
	GrB_Index  nrows  ;

	GxB_Matrix_type(&type, A) ;
	GxB_Type_size(&size, type) ;
	GrB_Matrix_nrows(&nrows, A) ;

	*iter = GB_MALLOC(1, GxB_MatrixTupleIter) ;
	GrB_Matrix_nvals(&((*iter)->nvals), A) ;

	(*iter)->A        =  A        ;
	(*iter)->nnz_idx  =  0        ;
	(*iter)->row_idx  =  0        ;
	(*iter)->nrows    =  nrows    ;
	(*iter)->p        =  A->p[0]  ;
	(*iter)->size     =  size     ;

	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_iterate_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_iterate_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;

	// Deplete iterator, should caller ignore returned error.
	_EmptyIterator(iter) ;

	if(rowIdx < 0 || rowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				 "Row index " GBu " out of range; must be < " GBu,
				 rowIdx, iter->nrows) ;
	}

	iter->p        =  0 ;
	iter->nvals    =  iter->A->p[rowIdx + 1] ;
	iter->nnz_idx  =  iter->A->p[rowIdx] ;
	iter->row_idx  =  rowIdx ;
	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_jump_to_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_jump_to_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;

	// Deplete iterator, should caller ignore returned error.
	_EmptyIterator(iter) ;

	if(rowIdx < 0 || rowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				 "Row index " GBu " out of range; must be < " GBu,
				 rowIdx, iter->nrows) ;
	}

	GrB_Matrix_nvals(&(iter->nvals), iter->A) ;

	iter->p        =  0                   ;
	iter->nnz_idx  =  iter->A->p[rowIdx]  ;
	iter->row_idx  =  rowIdx              ;

	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_iterate_range
(
	GxB_MatrixTupleIter *iter,  // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	GB_WHERE1("GxB_MatrixTupleIter_iterate_range (iter, startRowIdx, endRowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;

	// Deplete iterator, should caller ignore returned error.
	_EmptyIterator(iter) ;

	if(startRowIdx < 0 || startRowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				 "row index " GBu " out of range; must be < " GBu,
				 startRowIdx, iter->nrows) ;
	}

	if(startRowIdx > endRowIdx) {
		GB_ERROR(GrB_INVALID_INDEX,
				 "row index " GBu " must be > " GBu,
				 startRowIdx, endRowIdx) ;
	}

	iter->p        =  0                        ;
	iter->nnz_idx  =  iter->A->p[startRowIdx]  ;
	iter->row_idx  =  startRowIdx              ;
	if(endRowIdx < iter->nrows) iter->nvals = iter->A->p[endRowIdx + 1] ;
	else GrB_Matrix_nvals(&(iter->nvals), iter->A) ;

	return (GrB_SUCCESS) ;
}

// Advance iterator
GrB_Info GxB_MatrixTupleIter_next
(
	GxB_MatrixTupleIter *iter,      // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	void *val,                      // optional value at A[row,col]
	bool *depleted                  // indicate if iterator depleted
) {
	GB_WHERE1("GxB_MatrixTupleIter_next (iter, row, col, depleted)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL(depleted) ;
	GrB_Index nnz_idx = iter->nnz_idx ;

	if(nnz_idx >= iter->nvals) {
		*depleted = true ;
		return (GrB_SUCCESS) ;
	}

	GrB_Matrix A = iter->A ;

	//--------------------------------------------------------------------------
	// extract the column indices
	//--------------------------------------------------------------------------

	if(col)
		*col = A->i[nnz_idx] ;
	if(val) {
		GrB_Index offset = nnz_idx * iter->size;
		memcpy(val, (char*)A->x + offset, iter->size);
	}

	//--------------------------------------------------------------------------
	// extract the row indices
	//--------------------------------------------------------------------------

	const int64_t *Ap = A->p ;
	int64_t i = iter->row_idx ;

	for(; i < iter->nrows; i++) {
		int64_t p = iter->p + Ap[i] ;
		if(p < Ap[i + 1]) {
			iter->p++ ;
			if(row)
				*row = i ;
			break ;
		}
		iter->p = 0 ;
	}

	iter->row_idx = i ;

	iter->nnz_idx++ ;

	*depleted = false ;
	return (GrB_SUCCESS) ;
}

// Reset iterator
GrB_Info GxB_MatrixTupleIter_reset
(
	GxB_MatrixTupleIter *iter       // iterator to reset
) {
	GB_WHERE1("GxB_MatrixTupleIter_reset (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
	iter->nnz_idx  =  0              ;
	iter->row_idx  =  0              ;
	iter->p        =  iter->A->p[0]  ;
	return (GrB_SUCCESS) ;
}

// Update iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
	GxB_MatrixTupleIter *iter,      // iterator to update
	GrB_Matrix A                    // matrix to scan
) {
	GB_WHERE(A, "GxB_MatrixTupleIter_reuse (iter, A)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	// make sure matrix is not bitmap or full
	GxB_set(A, GxB_SPARSITY_CONTROL, GxB_SPARSE) ;

	size_t     size   ;
	GrB_Type   type   ;
	GrB_Index  nrows  ;

	GxB_Matrix_type(&type, A) ;
	GxB_Type_size(&size, type) ;
	GrB_Matrix_nrows(&nrows, A) ;

	iter->A      =  A      ;
	iter->size   =  size   ;
	iter->nrows  =  nrows  ;
	GrB_Matrix_nvals(&iter->nvals, A) ;
	GxB_MatrixTupleIter_reset(iter) ;
	return (GrB_SUCCESS) ;
}

// Release iterator
GrB_Info GxB_MatrixTupleIter_free
(
	GxB_MatrixTupleIter *iter       // iterator to free
) {
	GB_WHERE1("GxB_MatrixTupleIter_free (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_FREE(iter) ;
	return (GrB_SUCCESS) ;
}


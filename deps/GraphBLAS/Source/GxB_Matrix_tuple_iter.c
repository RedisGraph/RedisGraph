//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter,     // iterator to create
	GrB_Matrix A                    // matrix to iterate over
) {
	GB_WHERE("GxB_MatrixTupleIter_new (A)") ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	GrB_Index nrows;
	GrB_Matrix_nrows(&nrows, A);

	*iter = NULL ;
	GB_MALLOC_MEMORY(*iter, 1, sizeof(GxB_MatrixTupleIter)) ;
	GrB_Matrix_nvals(&((*iter)->nvals), A) ;
	(*iter)->A = A ;
	(*iter)->nnz_idx = 0 ;
	(*iter)->row_idx = 0 ;
	(*iter)->nrows = nrows;
	(*iter)->p = A->p[0] ;
	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_iterate_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE("GxB_MatrixTupleIter_iterate_row (iter, rowIdx)");
	GB_RETURN_IF_NULL(iter);

	if(rowIdx < 0 && rowIdx >= iter->nrows) {
		return (GB_ERROR(GrB_INVALID_INDEX, (GB_LOG, "Row index out of range")));
	}

	iter->nvals = iter->A->p[rowIdx + 1];
	iter->nnz_idx = iter->A->p[rowIdx];
	iter->row_idx = rowIdx;
	iter->p = 0;
	return (GrB_SUCCESS);
}

// Advance iterator
GrB_Info GxB_MatrixTupleIter_next
(
	GxB_MatrixTupleIter *iter,      // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	bool *depleted                  // indicate if iterator depleted
) {
	GB_WHERE("GxB_MatrixTupleIter_next (iter, row, col, depleted)") ;
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
		*col = A->i[nnz_idx];

	//--------------------------------------------------------------------------
	// extract the row indices
	//--------------------------------------------------------------------------

	const int64_t *Ap = A->p;
	int64_t i = iter->row_idx;

	for(; i < iter->nrows; i++) {
		int64_t p = iter->p + Ap[i];
		if(p < Ap[i + 1]) {
			iter->p++;
			if(row)
				*row = i;
			break;
		}
		iter->p = 0;
	}

	iter->row_idx = i;

	iter->nnz_idx++ ;

	*depleted = false ;
	return (GrB_SUCCESS) ;
}

// Reset iterator
GrB_Info GxB_MatrixTupleIter_reset
(
	GxB_MatrixTupleIter *iter       // iterator to reset
) {
	GB_WHERE("GxB_MatrixTupleIter_reset (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
	iter->nnz_idx = 0 ;
	iter->row_idx = 0 ;
	iter->p = iter->A->p[0] ;
	return (GrB_SUCCESS) ;
}

// Update iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
	GxB_MatrixTupleIter *iter,      // iterator to update
	GrB_Matrix A                    // matrix to scan
) {
	GB_WHERE("GxB_MatrixTupleIter_reuse (iter, A)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	GrB_Index nrows;
	GrB_Matrix_nrows(&nrows, A);

	iter->A = A ;
	iter->nrows = nrows ;
	GrB_Matrix_nvals(&iter->nvals, A) ;
	GxB_MatrixTupleIter_reset(iter) ;
	return (GrB_SUCCESS) ;
}

// Release iterator
GrB_Info GxB_MatrixTupleIter_free
(
	GxB_MatrixTupleIter *iter       // iterator to free
) {
	GB_WHERE("GxB_MatrixTupleIter_free (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_FREE_MEMORY(iter, 1, sizeof(GxB_MatrixTupleIter)) ;
	return (GrB_SUCCESS) ;
}

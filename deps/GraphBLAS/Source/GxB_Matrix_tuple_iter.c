//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// Sets iterator as depleted.
static inline void _EmptyIterator
(
	_GxB_MatrixTupleIter *iter   // Iterator to deplete.
) {
	iter->nvals = 0 ;
	iter->nnz_idx = 0 ;
}

GrB_Info GxB_MatrixTupleIter_init
(
	_GxB_MatrixTupleIter *iter,      // iterator to init
	GrB_Matrix A                    // matrix to iterate over
) {
	GB_WHERE(A, "GxB_MatrixTupleIter_init (iter, A)") ;
	GrB_Index nrows ;
	GrB_Matrix_nrows(&nrows, A) ;
	GrB_Matrix_nvals(&iter->nvals, A) ;

	GxB_Matrix_Option_get(A, GxB_SPARSITY_CONTROL, &iter->sparsity_type) ;
	if(iter->sparsity_type != GxB_SPARSE && iter->sparsity_type != GxB_HYPERSPARSE) {
		GB_ERROR (GrB_INVALID_VALUE,
			"Invalid sparsity type: %d", iter->sparsity_type) ;
	}
	iter->A = A ;
	iter->nnz_idx = 0 ;
	iter->row_idx = 0 ;
	iter->nrows = nrows ;
	iter->p = A->p[0] ;
	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		iter->sparse_row_idx = 0 ;
		iter->h_size = A->nvec ;
	}

	return GrB_SUCCESS ;
}

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter,     // list of iterators to create
	const GrB_Matrix A              // matrix to iterate over
) {
	GB_WHERE(A, "GxB_MatrixTupleIter_new (iter, A)") ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;
	*iter = GB_MALLOC(1, GxB_MatrixTupleIter) ;
	(*iter)->iterators_idx = 0;
	(*iter)->iterators_size = 1;

	return GxB_MatrixTupleIter_init(&(*iter)->iterators[0], A) ;
}

// finds the row index in Ah for HYPERSPARSE matrix,
// returns true if found else false.
static inline bool _find_row_index_in_Ah
(
	_GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	if(!result || !iter || !iter->h_size) return false ;
	GrB_Index l = 0, h = iter->h_size - 1, m ;
	// find the index using binary search
	while(l <= h) {
		m = (l + h)/2 ;
		int64_t val = iter->A->h[m] ;
		if(val == rowIdx) {
			*result = m ;
			return true ;
		} else if(val < rowIdx) {
			l = m + 1 ;
		} else { // val > rowIdx
			h = m - 1 ;
		}
	}

	return false ;
}

GrB_Info GxB_MatrixTupleIter_iterate_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_iterate_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;
	if(iter->iterators_size == 0) return GrB_INVALID_VALUE ;

	for(size_t i = 0; i < iter->iterators_size; i++) {
		_GxB_MatrixTupleIter *iter_cur = &iter->iterators[i] ;

		// Deplete iterator, should caller ignore returned error.
		_EmptyIterator(iter_cur) ;

		if(rowIdx < 0 || rowIdx >= iter_cur->nrows) {
			GB_ERROR(GrB_INVALID_INDEX,
					"Row index " GBu " out of range ; must be < " GBu,
					rowIdx, iter_cur->nrows) ;
		}

		GrB_Index rowIdx_sparse ;
		bool hypersparse_and_row_is_empty = false ;
		if(iter_cur->sparsity_type == GxB_SPARSE) {
			rowIdx_sparse = rowIdx ;
		} else {
			// GxB_HYPERSPARSE
			// locate row index is 'Ah'
			if(!_find_row_index_in_Ah(iter_cur, rowIdx, &rowIdx_sparse)) hypersparse_and_row_is_empty = true ;
		}

		// incase matrix is hyper-sparse and iterated row is empty, set 'nvals' to 0,
		// this will cause the next call to 'next' to report depleted
		iter_cur->nvals = hypersparse_and_row_is_empty ? 0 : iter_cur->A->p[rowIdx_sparse + 1] ;
		iter_cur->nnz_idx = iter_cur->A->p[rowIdx_sparse] ;
		iter_cur->row_idx = rowIdx ;
		if(iter_cur->sparsity_type == GxB_HYPERSPARSE) {
			iter_cur->sparse_row_idx = rowIdx_sparse ;
			iter_cur->h_size = iter_cur->A->nvec ;
		}
		iter_cur->p = 0 ;
	}

	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_jump_to_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_jump_to_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;
	if(iter->iterators_size == 0) return GrB_INVALID_VALUE ;

	for(size_t i = 0; i < iter->iterators_size; i++) {
		_GxB_MatrixTupleIter *iter_cur = &iter->iterators[i] ;

		// Deplete iterator, should caller ignore returned error.
		_EmptyIterator(iter_cur) ;

		if(rowIdx < 0 || rowIdx >= iter_cur->nrows) {
			GB_ERROR(GrB_INVALID_INDEX,
					"Row index " GBu " out of range ; must be < " GBu,
					rowIdx, iter_cur->nrows) ;
		}

		GrB_Index _rowIdx = rowIdx ;
		if(iter_cur->sparsity_type == GxB_HYPERSPARSE) {
			GrB_Index rowIdx_sparse ;
			if(!_find_row_index_in_Ah(iter_cur, rowIdx, &rowIdx_sparse)) {
				GB_ERROR (GrB_INVALID_INDEX,
					"Row index " GBu " doesn't exist in the hypersparse matrix, row might be empty",
					rowIdx ) ;
			}
			_rowIdx = rowIdx_sparse ;
			iter_cur->sparse_row_idx = rowIdx_sparse ;
			iter_cur->h_size = iter_cur->A->nvec ;
		}
		GrB_Matrix_nvals(&(iter_cur->nvals), iter_cur->A) ;

		iter_cur->p        =  0                   ;
		iter_cur->nnz_idx  =  iter_cur->A->p[_rowIdx]  ;
		iter_cur->row_idx  =  rowIdx              ;
	}

	return (GrB_SUCCESS) ;
}

// finds the start row index in Ah for HYPERSPARSE matrix, 
// returns true if found else false.
static inline bool _find_start_row_index_in_Ah
(
	_GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	GB_WHERE1("_find_start_row_index_in_Ah (iter, rowIdx, result)") ;
	if(!iter->h_size) return false ;
	GrB_Matrix A = iter->A ;
	bool found ;
	GrB_Index l = 0, r = iter->h_size - 1 ;

	GB_BINARY_SEARCH(rowIdx, A->h, l, r, found) ;
	if(found) {
		*result = l ;
		return found ;
	} else if(A->h[l] > rowIdx) {
		// rowIdx not found, look for the minimal row index which is greater than rowIdx
		// this can be located in h[l] or h[l+1]
		*result = l ;
		return true ;
	} else if(l + 1 < iter->h_size) {
		if(A->h[l + 1] <= rowIdx) {
			GB_ERROR(GrB_INVALID_VALUE,
				"row index " GBu " must be > than rowIdx" GBu,
				A->h[l + 1], rowIdx) ;
		}
		*result = l+1 ;
		return true ;
	}

	// reaching here means no start row index found
	return false ;
}

// finds the end row index in Ah for HYPERSPARSE matrix,
// returns true if found else false.
static inline bool _find_end_row_index_in_Ah
(
	_GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	GB_WHERE1("_find_end_row_index_in_Ah (iter, rowIdx, result)") ;
	if(!iter->h_size) return false ;
	GrB_Matrix A = iter->A ;
	bool found ;
	GrB_Index l = 0, r = iter->h_size - 1 ;

	GB_BINARY_SEARCH(rowIdx, A->h, l, r, found) ;
	if(found) {
		*result = l ;
		return found ;
	} else if(A->h[l] < rowIdx) {
		// rowIdx not found, look for the maximal row index which is smaller than rowIdx
		// this can be located in h[l] or h[l+1]
		*result = l ;
		return true ;
	} else if(l > 0) {
		if(A->h[l - 1] >= rowIdx) {
			GB_ERROR(GrB_INVALID_VALUE,
				"row index " GBu " must be < than rowIdx" GBu,
				A->h[l + 1], rowIdx) ;
		}
		*result = l - 1 ;
		return true ;
	}

	// reaching here means no start row index found
	return false ;
}

GrB_Info GxB_MatrixTupleIter_iterate_range
(
	GxB_MatrixTupleIter *iter,  // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) {
	GB_WHERE1("GxB_MatrixTupleIter_iterate_range (iter, startRowIdx, endRowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;
	if(iter->iterators_size == 0) return GrB_INVALID_VALUE ;

	for(size_t i = 0; i < iter->iterators_size; i++) {
		_GxB_MatrixTupleIter *iter_cur = &iter->iterators[i] ;

		// Deplete iterator, should caller ignore returned error.
		_EmptyIterator(iter_cur) ;

		if(startRowIdx < 0 || startRowIdx >= iter_cur->nrows) {
			GB_ERROR(GrB_INVALID_INDEX,
					"row index " GBu " out of range ; must be < " GBu,
					startRowIdx, iter_cur->nrows) ;
		}

		if(startRowIdx > endRowIdx) {
			GB_ERROR(GrB_INVALID_INDEX,
					"row index " GBu " must be > " GBu,
					startRowIdx, endRowIdx) ;
		}

		GrB_Index startRowIdx_sparse = startRowIdx ;
		GrB_Index endRowIdx_sparse = endRowIdx ;
		bool hypersparse_no_more_rows = false ;

		if(iter_cur->sparsity_type == GxB_HYPERSPARSE) {
			iter_cur->h_size = iter_cur->A->nvec ;
			if(!_find_start_row_index_in_Ah(iter_cur, startRowIdx, &startRowIdx_sparse)
			|| !_find_end_row_index_in_Ah(iter_cur, endRowIdx, &endRowIdx_sparse)
			|| startRowIdx > endRowIdx) // In case Ah has no variables in the range but have variables on both sides of the range.
				hypersparse_no_more_rows = true ;
			iter_cur->sparse_row_idx = startRowIdx_sparse ;
		}
		iter_cur->p       =  0 ;
		iter_cur->nnz_idx = iter_cur->A->p[startRowIdx_sparse] ;
		iter_cur->row_idx = startRowIdx ;
		if(hypersparse_no_more_rows) iter_cur->nvals = 0 ; // simulate depletion of the iterator
		else if(endRowIdx_sparse < iter_cur->nrows) iter_cur->nvals = iter_cur->A->p[endRowIdx_sparse + 1] ;
		else GrB_Matrix_nvals(&(iter_cur->nvals), iter_cur->A) ;
	}

	return (GrB_SUCCESS) ;
}

// Advance iterator
GrB_Info GxB_MatrixTupleIter_next
(
	GxB_MatrixTupleIter *iter,      // iterator to consume
	GrB_Index *row,                 // optional output row index
	GrB_Index *col,                 // optional output column index
	bool *depleted                  // indicate if iterator depleted
) {
	GB_WHERE1("GxB_MatrixTupleIter_next (iter, row, col, depleted)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL(depleted) ;
	_GxB_MatrixTupleIter *iter_cur = &iter->iterators[iter->iterators_idx] ;
	GrB_Index nnz_idx = iter_cur->nnz_idx ;

	if(nnz_idx >= iter_cur->nvals) { // current iterator depleted start consuming next iterator
		if(iter->iterators_idx + 1 >= iter->iterators_size) {
			*depleted = true ;
			return (GrB_SUCCESS) ;
		} else {
			iter->iterators_idx++ ;
			iter_cur = &iter->iterators[iter->iterators_idx];
			nnz_idx = iter_cur->nnz_idx ;
		}
	}

	GrB_Matrix A = iter_cur->A ;

	//--------------------------------------------------------------------------
	// extract the column indices
	//--------------------------------------------------------------------------

	if(col)
		*col = A->i[nnz_idx] ;

	//--------------------------------------------------------------------------
	// extract the row indices
	//--------------------------------------------------------------------------

	const int64_t *Ap = A->p ;
	int64_t i ;
	GrB_Index nrows ;
	if(iter_cur->sparsity_type == GxB_SPARSE) {
		i = iter_cur->row_idx ;
		nrows = iter_cur->nrows ;
	} else { // GxB_HYPERSPARSE
		i = iter_cur->sparse_row_idx ;
		nrows = iter_cur->h_size ;
	}

	for( ; i < nrows ; i++) {
		int64_t p = iter_cur->p + Ap[i] ;
		if(p < Ap[i + 1]) {
			iter_cur->p++ ;
			if(row)
				*row = (iter_cur->sparsity_type == GxB_SPARSE) ? i : A->h[i] ;
			break ;
		}
		iter_cur->p = 0 ;
	}

	if(iter_cur->sparsity_type == GxB_SPARSE) {
		iter_cur->row_idx = i ;
	} else { // GxB_HYPERSPARSE
		iter_cur->row_idx = A->h[i] ;
		iter_cur->sparse_row_idx = i ;
	}

	iter_cur->nnz_idx++ ;

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
	if(iter->iterators_size == 0) return GrB_INVALID_VALUE ;
	iter->iterators_idx = 0;

	GrB_Info rv = GrB_SUCCESS ;
	for(size_t i = 0; i < iter->iterators_size; i++)
		rv |= GxB_MatrixTupleIter_init(&iter->iterators[i], iter->iterators[i].A) ;

	return rv ;
}

// Update iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
	GxB_MatrixTupleIter *iter,      // iterator to update
	GrB_Matrix A                    // matrix to scan
) {
	GB_RETURN_IF_NULL(iter) ;
 	GB_RETURN_IF_NULL(A) ;

	iter->iterators_idx = 0 ;
	iter->iterators_size = 1 ;

	// make sure matrix is not bitmap or full
	GxB_set(A, GxB_SPARSITY_CONTROL, GxB_SPARSE) ; // TODO: remove this

	return GxB_MatrixTupleIter_init(&iter->iterators[0], A) ;
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


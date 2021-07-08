//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// in hypersparse matrix the Ah is sparse so need to save the idx and the size of this array
static inline void _MatrixTupleIter_init_hypersparse_fields
(
	_GxB_MatrixTupleIter *iter,      // iterator to init
	int64_t sparse_row_idx,          // index into hyper-sparse row array 'h'
    int64_t h_size                   // Number of entries in hyper-sparse row array 'h'
) {
	iter->sparse_row_idx = sparse_row_idx;
	iter->h_size = h_size;
}

// Sets iterator as depleted.
static inline void _EmptyIterator
(
	_GxB_MatrixTupleIter *iter   // Iterator to deplete.
) {
	iter->nvals = 0 ;
	iter->nnz_idx = 0 ;
}

static inline GrB_Info _MatrixTupleIter_init
(
	_GxB_MatrixTupleIter *iter,      // iterator to init
	GrB_Matrix A,                    // matrix to iterate over
	int sparsity_type                // The sparsity type of the matrix
) {
	GB_WHERE(A, "_MatrixTupleIter_init (iter, A)") ;
	GrB_Index nrows ;
	GrB_Matrix_nrows(&nrows, A) ;
	GrB_Matrix_nvals(&iter->nvals, A) ;

	iter->sparsity_type = sparsity_type;
	iter->A = A ;
	iter->nnz_idx = 0 ;
	iter->row_idx = 0 ;
	iter->nrows = nrows ;
	iter->p = A->p[0] ;
	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		_MatrixTupleIter_init_hypersparse_fields(iter, 0, A->nvec);
	}

	return GrB_SUCCESS ;
}

// finds the start row index in Ah for HYPERSPARSE matrix,
// returns true if found else false.
static inline bool _find_minimal_row_in_Ah_greater_or_equal_to_rowIdx
(
	_GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	GB_WHERE1("_find_minimal_row_in_Ah_greater_or_equal_to_rowIdx (iter, rowIdx, result)") ;
	if(!iter->h_size) return false ; // no rows in Ah return not found
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

	// row index matching criteria wasn't found
	return false ;
}

// finds the end row index in Ah for HYPERSPARSE matrix,
// returns true if found else false.
static inline bool _find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx
(
	_GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	GB_WHERE1("_find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx (iter, rowIdx, result)") ;
	if(!iter->h_size) return false ; // no rows in Ah return not found
	GrB_Matrix A = iter->A ;
	bool found ;
	GrB_Index l = 0, r = iter->h_size - 1 ;

	GB_BINARY_SEARCH(rowIdx, A->h, l, r, found) ;
	if(found) {
		*result = l ;
		return found ;
	} else if(A->h[l] < rowIdx) {
		// rowIdx not found, look for the maximal row index which is smaller than rowIdx
		// this can be located in h[l] or h[l-1]
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

	// row index matching criteria wasn't found
	return false ;
}

// finds the row index in Ah for HYPERSPARSE matrix,
// returns true if found else false.
static inline bool _find_row_index_in_Ah
(
	const _GxB_MatrixTupleIter *iter,  // the iterator
	GrB_Index rowIdx,                  // the row index to look for in the matrix
	GrB_Index *result                  // the index in Ah in which the row index located
) {
	GB_RETURN_IF_NULL(result) ;
	GB_RETURN_IF_NULL(iter) ;
	if(iter->h_size == 0) return false ; // no rows in Ah return not found
	GrB_Index l = 0 ;
	GrB_Index h = iter->h_size - 1 ;
	GrB_Index m ;
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

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter,     // iterator to create
	const GrB_Matrix A              // matrix to iterate over
) {
	GB_WHERE(A, "GxB_MatrixTupleIter_new (iter, A)") ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;
	int sparsity_type;
	GxB_Matrix_Option_get(A, GxB_SPARSITY_CONTROL, &sparsity_type) ;
	if(sparsity_type != GxB_SPARSE && sparsity_type != GxB_HYPERSPARSE)
		GB_ERROR (GrB_INVALID_VALUE, "Invalid sparsity type: %d", sparsity_type) ;

	*iter = GB_MALLOC(1, GxB_MatrixTupleIter) ;
	(*iter)->idx = 0;
	(*iter)->n = 1;

	_GxB_MatrixTupleIter *it = (*iter)->iterators;
	GrB_Info info = _MatrixTupleIter_init(it, A, sparsity_type) ;
	return info;
}

GrB_Info GxB_MatrixTupleIter_iterate_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_iterate_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;
	if(iter->n == 0) return GrB_INVALID_VALUE ; // no iterators to iterate over is invalid scenario

	for(size_t i = 0; i < iter->n; i++) {
		_GxB_MatrixTupleIter *iter_cur = &iter->iterators[i] ;

		// Deplete iterator, should caller ignore returned error.
		_EmptyIterator(iter_cur) ;

		if(rowIdx < 0 || rowIdx >= iter_cur->nrows) {
			GB_ERROR(GrB_INVALID_INDEX,
					"Row index " GBu " out of range ; must be < " GBu,
					rowIdx, iter_cur->nrows) ;
		}

		GrB_Index _rowIdx ;
		bool hypersparse_and_row_is_empty = false ;
		if(iter_cur->sparsity_type == GxB_SPARSE) {
			_rowIdx = rowIdx ;
		} else {
			// GxB_HYPERSPARSE
			// locate row index is 'Ah'
			if(!_find_row_index_in_Ah(iter_cur, rowIdx, &_rowIdx)) hypersparse_and_row_is_empty = true ;
		}

		// incase matrix is hyper-sparse and iterated row is empty, set 'nvals' to 0,
		// this will cause the next call to 'next' to report depleted
		iter_cur->nvals = hypersparse_and_row_is_empty ? 0 : iter_cur->A->p[_rowIdx + 1] ;
		iter_cur->nnz_idx = iter_cur->A->p[_rowIdx] ;
		iter_cur->row_idx = rowIdx ;
		if(iter_cur->sparsity_type == GxB_HYPERSPARSE)
			_MatrixTupleIter_init_hypersparse_fields(iter_cur, _rowIdx, iter_cur->A->nvec);
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
	if(iter->n == 0) return GrB_INVALID_VALUE ; // no iterators to iterate over is invalid scenario

	for(size_t i = 0; i < iter->n; i++) {
		_GxB_MatrixTupleIter *iter_cur = &iter->iterators[i] ;

		// Deplete iterator, should caller ignore returned error.
		_EmptyIterator(iter_cur) ;

		if(rowIdx < 0 || rowIdx >= iter_cur->nrows) {
			GB_ERROR(GrB_INVALID_INDEX,
					"Row index " GBu " out of range ; must be < " GBu,
					rowIdx, iter_cur->nrows) ;
		}

		GrB_Index _rowIdx = rowIdx ; // the normalized rowIdx
		if(iter_cur->sparsity_type == GxB_HYPERSPARSE) {
			if(!_find_row_index_in_Ah(iter_cur, rowIdx, &_rowIdx)) { // In hypersparse _rowIdx should be the index to Ah
				GB_ERROR (GrB_INVALID_INDEX,
					"Row index " GBu " doesn't exist in the hypersparse matrix, row might be empty",
					rowIdx ) ;
			}
			_MatrixTupleIter_init_hypersparse_fields(iter_cur, _rowIdx, iter_cur->A->nvec);
		}
		GrB_Matrix_nvals(&(iter_cur->nvals), iter_cur->A) ;

		iter_cur->p        =  0                   ;
		iter_cur->nnz_idx  =  iter_cur->A->p[_rowIdx]  ;
		iter_cur->row_idx  =  rowIdx              ;
	}

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
	if(iter->n == 0) return GrB_INVALID_VALUE ; // no iterators to iterate over is invalid scenario

	for(size_t i = 0; i < iter->n; i++) {
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

		GrB_Index _startRowIdx = startRowIdx ;
		GrB_Index _endRowIdx = endRowIdx ;
		bool hypersparse_no_more_rows = false ;

		if(iter_cur->sparsity_type == GxB_HYPERSPARSE) {
			if(!_find_minimal_row_in_Ah_greater_or_equal_to_rowIdx(iter_cur, startRowIdx, &_startRowIdx)
			|| !_find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx(iter_cur, endRowIdx, &_endRowIdx)
			|| _startRowIdx > _endRowIdx) { // In case Ah has no variables in the range but have variables on both sides of the range.
				hypersparse_no_more_rows = true ;
			}
			_MatrixTupleIter_init_hypersparse_fields(iter_cur, _startRowIdx, iter_cur->A->nvec);
		}
		iter_cur->p       =  0 ;
		iter_cur->nnz_idx = iter_cur->A->p[_startRowIdx] ;
		iter_cur->row_idx = startRowIdx ;
		if(hypersparse_no_more_rows) iter_cur->nvals = 0 ; // simulate depletion of the iterator
		else if(_endRowIdx < iter_cur->nrows) iter_cur->nvals = iter_cur->A->p[_endRowIdx + 1] ;
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
	_GxB_MatrixTupleIter *iter_cur = &iter->iterators[iter->idx] ;
	GrB_Index nnz_idx = iter_cur->nnz_idx ;

	if(nnz_idx >= iter_cur->nvals) { // current iterator depleted start consuming next iterator
		if(iter->idx + 1 >= iter->n) {
			*depleted = true ;
			return (GrB_SUCCESS) ;
		} else {
			iter->idx++ ;
			iter_cur = &iter->iterators[iter->idx];
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

	// update the current row_idx in the iterator
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
	if(iter->n == 0) return GrB_INVALID_VALUE ; // no iterators to iterate over is invalid scenarios
	iter->idx = 0;

	GrB_Info rv = GrB_SUCCESS ;
	for(size_t i = 0; i < iter->n; i++) {
		int sparsity_type;
		GxB_Matrix_Option_get(iter->iterators[i].A, GxB_SPARSITY_CONTROL, &sparsity_type) ;
		if(sparsity_type != GxB_SPARSE && sparsity_type != GxB_HYPERSPARSE)
			GB_ERROR (GrB_INVALID_VALUE, "Invalid sparsity type: %d", sparsity_type) ;
		if((rv = _MatrixTupleIter_init(&iter->iterators[i], iter->iterators[i].A, sparsity_type)))
			return rv;
	}

	return rv ;
}

// Update iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
	GxB_MatrixTupleIter *iter,      // iterator to update
	GrB_Matrix A                    // matrix to scan
) {
	GB_WHERE1("GxB_MatrixTupleIter_reuse (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
 	GB_RETURN_IF_NULL(A) ;

	iter->idx = 0 ;
	iter->n = 1 ;

	int sparsity_type;
	GxB_Matrix_Option_get(A, GxB_SPARSITY_CONTROL, &sparsity_type) ;
	if(sparsity_type != GxB_SPARSE && sparsity_type != GxB_HYPERSPARSE)
		GB_ERROR (GrB_INVALID_VALUE, "Invalid sparsity type: %d", sparsity_type) ;

	return _MatrixTupleIter_init(&iter->iterators[0], A, sparsity_type) ;
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


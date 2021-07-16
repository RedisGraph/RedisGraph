//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// in hypersparse matrix the Ah is sparse so need to save the idx and the size
// of this array
static inline void _MatrixTupleIter_init_hypersparse_fields
(
	GxB_MatrixTupleIter *iter,  // iterator to init
	int64_t sparse_row_idx      // index into hyper-sparse array 'h'
) {
	iter->h_size          =  iter->A->nvec ;
	iter->sparse_row_idx  =  sparse_row_idx ;
}

// sets iterator as depleted
static inline void _EmptyIterator
(
	GxB_MatrixTupleIter *iter   // iterator to deplete
) {
	iter->nvals = 0 ;
	iter->nnz_idx = 0 ;
}

static GrB_Info _MatrixTupleIter_init
(
	GxB_MatrixTupleIter *iter,      // iterator to init
	const GrB_Matrix A              // matrix to iterate over
) {
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	size_t     size   ;
	GrB_Type   type   ;
	GrB_Index  nrows  ;
	int sparsity_type ;

	GxB_Matrix_type(&type, A) ;
	GxB_Type_size(&size, type) ;

	GrB_Matrix_nrows(&nrows, A) ;
	GrB_Matrix_nvals(&(iter->nvals), A) ;

	GxB_Matrix_Option_get(A, GxB_SPARSITY_STATUS, &sparsity_type);
	  
	iter->A              =  A              ;
	iter->p              =  A->p[0]        ;
	iter->size           =  size           ;
	iter->nrows          =  nrows          ;
	iter->nnz_idx        =  0              ;
	iter->row_idx        =  0              ;
	iter->sparsity_type  =  sparsity_type  ;
  
	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		_MatrixTupleIter_init_hypersparse_fields(iter, 0) ;
  }

	return (GrB_SUCCESS) ;
}

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter,     // iterator to create
	const GrB_Matrix A              // matrix to iterate over
 ) {
	GB_WHERE(A, "GxB_MatrixTupleIter_new (A)") ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	*iter = GB_MALLOC(1, GxB_MatrixTupleIter) ;
	return _MatrixTupleIter_init(*iter, A) ;
}

// find the start row index in Ah for HYPERSPARSE matrix,
// return true if found false otherwise
static bool _find_minimal_row_in_Ah_greater_or_equal_to_rowIdx
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	if(!iter->h_size) return false ; // empty matrix, no rows in Ah return not found

	bool found ;
	GrB_Matrix  A  =  iter->A          ;
	GrB_Index   l  =  0                ;
	GrB_Index   r  =  iter->h_size - 1 ;

	GB_BINARY_SEARCH(rowIdx, A->h, l, r, found) ;

	if(found) {
		*result = l ;
	} else {
		// not found
		if(A->h[l] > rowIdx) {
			// rowIdx not found, look for the minimal row index which is greater than rowIdx
			// this can be located in h[l] or h[l+1]
			*result = l ;
			found = true ;
		} else if(l + 1 < iter->h_size) {
			// the value at l+1 if exist must be greater than rowIdx, by the GB_BINARY_SEARCH promises
			ASSERT(A->h[l + 1] > rowIdx);
			*result = l+1 ;
			found = true ;
		}
	}

	return found ;
}

// find the end row index in Ah for HYPERSPARSE matrix,
// return true if found false otherwise
static bool _find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx,
	GrB_Index *result
) {
	if(!iter->h_size) return false ; // no rows in Ah return not found

	bool found ;
	GrB_Matrix  A  =  iter->A          ;
	GrB_Index   l  =  0                ;
	GrB_Index   r  =  iter->h_size - 1 ;

	GB_BINARY_SEARCH(rowIdx, A->h, l, r, found) ;

	if(found) {
		*result = l ;
	} else {
		// not found
		if(A->h[l] < rowIdx) {
			// rowIdx not found, look for the maximal row index which is smaller than rowIdx
			// this can be located in h[l] or h[l-1]
			*result = l ;
			found = true ;
		} else if(l > 0) {
			// the value at l-1 if exist must be smaller than rowIdx, by the GB_BINARY_SEARCH promises
			ASSERT(A->h[l - 1] < rowIdx);
			*result = l - 1 ;
			found = true ;
		}
	}

	return found ;
}

// find the row index in Ah for HYPERSPARSE matrix by doing simple binary search,
// return true if found false otherwise
static bool _find_row_index_in_Ah
(
	const GxB_MatrixTupleIter *iter,  // the iterator
	GrB_Index rowIdx,                 // the row index to look for in the matrix
	GrB_Index *result                 // the index in Ah in which the row index located
) {
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL(result) ;

	if(iter->h_size == 0) return false ; // empty matrix, no rows in Ah, return not found

	GrB_Index m ;
	GrB_Index l = 0 ;
	GrB_Index r = iter->h_size - 1 ;

	// find the index using binary search
	while(l <= r) {
		m = (l + r) / 2 ;
		int64_t val = iter->A->h[m] ;

		if(val == rowIdx) {
			*result = m ;
			return true ;
		} else if(val < rowIdx) {
			l = m + 1 ;
		} else { // val > rowIdx
			r = m - 1 ;
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

	// deplete iterator, should caller ignore returned error
	_EmptyIterator(iter) ;

	if(rowIdx < 0 || rowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				"Row index " GBu " out of range ; must be < " GBu,
				rowIdx, iter->nrows) ;
	}

	GrB_Index _rowIdx ;
	bool hypersparse_and_row_is_empty = false ;

	if(iter->sparsity_type == GxB_SPARSE) {
		_rowIdx = rowIdx ;
	} else {
		// GxB_HYPERSPARSE
		// locate row index is 'Ah'
		if(!_find_row_index_in_Ah(iter, rowIdx, &_rowIdx)) {
			hypersparse_and_row_is_empty = true ;
		}
	}

	// incase matrix is hyper-sparse and iterated row is empty, set 'nvals' to 0,
	// this will cause the next call to 'next' to report depleted
	if(hypersparse_and_row_is_empty) {
		iter->nvals    =  0 ;
		iter->nnz_idx  =  0 ;
		iter->row_idx  =  0 ;
	} else {
		iter->nvals    =  iter->A->p[_rowIdx + 1] ;
		iter->nnz_idx  =  iter->A->p[_rowIdx] ;
		iter->row_idx  =  rowIdx ;

		if(iter->sparsity_type == GxB_HYPERSPARSE) {
			_MatrixTupleIter_init_hypersparse_fields(iter, _rowIdx) ;
		}
	}

	iter->p = 0 ;

	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_jump_to_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_jump_to_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;

	// deplete iterator, should caller ignore returned error
	_EmptyIterator(iter) ;

	if(rowIdx < 0 || rowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				"Row index " GBu " out of range ; must be < " GBu,
				rowIdx, iter->nrows) ;
	}

	// this call needed because we deplete the iterator
	GrB_Matrix_nvals(&(iter->nvals), iter->A) ;

	GrB_Index _rowIdx = rowIdx ; // the normalized rowIdx

	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		if(!_find_row_index_in_Ah(iter, rowIdx, &_rowIdx)) { // In hypersparse _rowIdx should be the index to Ah
			GB_ERROR (GrB_INVALID_INDEX,
				"Row index " GBu " doesn't exist in the hypersparse matrix, row might be empty",
				rowIdx ) ;
		}
		_MatrixTupleIter_init_hypersparse_fields(iter, _rowIdx) ;
	}

	iter->p        =  0 ;
	iter->nnz_idx  =  iter->A->p[_rowIdx] ;
	iter->row_idx  =  rowIdx ;

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

	if(startRowIdx > endRowIdx) {
		GB_ERROR(GrB_INVALID_INDEX,
				"row index " GBu " must be > " GBu,
				startRowIdx, endRowIdx) ;
	}

	if(startRowIdx < 0 || startRowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				"row index " GBu " out of range ; must be < " GBu,
				startRowIdx, iter->nrows) ;
	}

	GrB_Index  _startRowIdx          =  startRowIdx  ;
	GrB_Index  _endRowIdx            =  endRowIdx    ;
	bool       hypersparse_has_rows  =  true         ;

	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		bool start_idx_found =
			_find_minimal_row_in_Ah_greater_or_equal_to_rowIdx(iter,
					startRowIdx, &_startRowIdx) ;

		bool end_idx_found =
			_find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx(iter, endRowIdx,
					&_endRowIdx) ;

		// it is possible for _startRowIdx to be greater than _endRowIdx
		// consider the sparse array: [10,20,30]
		// startRowIdx = 15
		// endRowIDx   = 16
		// in this case _startRowIdx = 20 and _endRowIdx = 10

		hypersparse_has_rows = (start_idx_found &&
				                end_idx_found &&
								_startRowIdx <= _endRowIdx);

		if(hypersparse_has_rows == true) {
				_MatrixTupleIter_init_hypersparse_fields(iter, _startRowIdx) ;
		}
	}

	iter->p       =  0 ;
	iter->nnz_idx = iter->A->p[_startRowIdx] ;
	iter->row_idx = startRowIdx ;
	if(!hypersparse_has_rows) {
		// simulate depletion of the iterator
		iter->nvals = 0 ; 
	} else if(_endRowIdx < iter->nrows) {
		iter->nvals = iter->A->p[_endRowIdx + 1] ;
	} else {
		GrB_Matrix_nvals(&(iter->nvals), iter->A) ;
	}

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
	int64_t i ;
	GrB_Index nrows ;
	if(iter->sparsity_type == GxB_SPARSE) {
		i = iter->row_idx ;
		nrows = iter->nrows ;
	} else { // GxB_HYPERSPARSE
		i = iter->sparse_row_idx ;
		nrows = iter->h_size ;
	}

	for( ; i < nrows ; i++) {
		int64_t p = iter->p + Ap[i] ;
		// the number of columns in a row equals to Ap[i+1] - Ap[i] 
		// thus if p == Ap[i+1] means we exhausted the current row.
		if(p < Ap[i + 1]) {
			iter->p++ ;
			if(row) {
				*row = (iter->sparsity_type == GxB_SPARSE) ? i : A->h[i] ;
			}
			break ;
		}
		iter->p = 0 ;
	}

	// update the current row_idx in the iterator
	if(iter->sparsity_type == GxB_SPARSE) {
		iter->row_idx = i ;
	} else { // GxB_HYPERSPARSE
		iter->row_idx = A->h[i] ;  // in hypersparse scenario the row index determined by the value at Ah[i]
		iter->sparse_row_idx = i ;
	}

	iter->nnz_idx++ ;

	*depleted = false ;
	return (GrB_SUCCESS) ;
}

// Reset iterator, assumes the iterator is valid
GrB_Info GxB_MatrixTupleIter_reset
(
	GxB_MatrixTupleIter *iter       // iterator to reset
) {
	GB_WHERE1("GxB_MatrixTupleIter_reset (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
	
	return _MatrixTupleIter_init(iter, iter->A) ;
}

// Update iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
	GxB_MatrixTupleIter *iter,      // iterator to update
	const GrB_Matrix A              // matrix to scan
) {
	GB_WHERE1("GxB_MatrixTupleIter_reuse (iter)") ;
 	GB_RETURN_IF_NULL(A) ;
	GB_RETURN_IF_NULL(iter) ;

	int sparsity_type;
	GxB_Matrix_Option_get(A, GxB_SPARSITY_STATUS, &sparsity_type) ;
	if(sparsity_type != GxB_SPARSE && sparsity_type != GxB_HYPERSPARSE)
		GB_ERROR (GrB_INVALID_VALUE, "Invalid sparsity type: %d", sparsity_type) ;

	return _MatrixTupleIter_init(iter, A) ;
}

// release iterator
GrB_Info GxB_MatrixTupleIter_free
(
	GxB_MatrixTupleIter *iter       // iterator to free
) {
	GB_WHERE1("GxB_MatrixTupleIter_free (iter)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_FREE(iter) ;
	return (GrB_SUCCESS) ;
}


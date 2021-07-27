//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// sets iterator as depleted
static inline void _EmptyIterator
(
	GxB_MatrixTupleIter *iter   // iterator to deplete
) {
	iter->nvals   = 0 ;
	iter->nnz_idx = 0 ;
}

static GrB_Info _init
(
	GxB_MatrixTupleIter *iter,      // iterator to init
	const GrB_Matrix A              // matrix to iterate over
) {
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	size_t     size   ;
	GrB_Type   type   ;
	GrB_Index  nrows  ;
	GrB_Index  nvals  ;
	int sparsity_type ;

	GxB_Matrix_type(&type, A) ;
	GxB_Type_size(&size, type) ;

	GrB_Matrix_nvals(&nvals, A) ;
	GrB_Matrix_nrows(&nrows, A) ;

	GxB_Matrix_Option_get(A, GxB_SPARSITY_STATUS, &sparsity_type) ;

	iter->A              =  A              ;
	iter->p              =  0              ;
	iter->size           =  size           ;
	iter->nrows          =  nrows          ;
	iter->nvals          =  nvals          ;
	iter->nnz_idx        =  0              ;
	iter->row_idx        =  0              ;
	iter->sparsity_type  =  sparsity_type  ;

	return (GrB_SUCCESS) ;
}

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter,     // iterator to create
	const GrB_Matrix A              // matrix to iterate over
 ) {
	GB_WHERE(A, "GxB_MatrixTupleIter_new (A)") ;
	GB_RETURN_IF_NULL(iter) ;
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;

	int sparsity ;
	GxB_Matrix_Option_get(A, GxB_SPARSITY_STATUS, &sparsity) ;
	if(sparsity != GxB_SPARSE && sparsity != GxB_HYPERSPARSE) {
		GB_ERROR(GrB_INVALID_VALUE, "Invalid sparsity type: %d", sparsity) ;
	}

	*iter = GB_MALLOC(1, GxB_MatrixTupleIter) ;
	return _init(*iter, A) ;
}

// find the start row index in Ah for HYPERSPARSE matrix,
// return true if found false otherwise
static bool _find_minimal_row_in_Ah_greater_or_equal_to_rowIdx
(
	const GrB_Matrix A,
	GrB_Index i,
	GrB_Index *result
) {
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

	GB_RETURN_IF_NULL(A) ;
	GB_RETURN_IF_NULL(result) ;

	if(A->nvec == 0) return false;

	bool found ;
	GrB_Index   left  =  0 ;
	GrB_Index   right =  A->nvec - 1 ;
	const GrB_Index *__restrict__ Ah = (GrB_Index *)A->h ;

	GB_BINARY_SEARCH(i, Ah, left, right, found) ;

	if(found) {
		*result = left ;
	} else {
		// not found
		if(Ah[left] > i) {
			// i not found, look for the minimal value which is greater than i
			// this can be located in Ah[left] or Ah[left+1]
			*result = left ;
			found = true ;
		} else if(left + 1 < A->nvec) {
			// the value at l+1 if exist must be greater than rowIdx, by the GB_BINARY_SEARCH promises
			ASSERT(Ah[left + 1] > i);
			*result = left+1 ;
			found = true ;
		}
	}

	return found ;
}

// find the end row index in Ah for HYPERSPARSE matrix,
// return true if found false otherwise
static bool _find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx
(
	const GrB_Matrix A,
	GrB_Index i,
	GrB_Index *result
) {
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

	GB_RETURN_IF_NULL(A) ;
	GB_RETURN_IF_NULL(result) ;

	if(A->nvec == 0) return false;

	bool found ;
	GrB_Index   left  =  0 ;
	GrB_Index   right =  A->nvec - 1 ;
	const GrB_Index *__restrict__ Ah = (GrB_Index *)A->h ;

	GB_BINARY_SEARCH(i, Ah, left, right, found) ;

	if(found) {
		*result = left ;
	} else {
		// not found
		if(Ah[left] < i) {
			GxB_print(A, GxB_COMPLETE_VERBOSE);
			// i not found, look for the maximal value which is smaller than i
			// this can be located in Ah[left] or Ah[left-1]
			*result = left ;
			found = true ;
		} else if(left > 0) {
			// the value at l-1 if exist must be smaller than rowIdx, by the GB_BINARY_SEARCH promises
			ASSERT(Ah[left - 1] < i);
			*result = left - 1 ;
			found = true ;
		}
	}

	return found ;
}

// find the row in Ah by doing simple binary search,
// return true if found false otherwise
static bool _find_row_index_in_Ah
(
	const GrB_Matrix A,  // matrix to search
	GrB_Index i,         // row index to locate
	GrB_Index *result    // position of i in Ah
) {
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

	GB_RETURN_IF_NULL(A) ;
	GB_RETURN_IF_NULL(result) ;
	
	if(A->nvec == 0) return false;

	bool found ;
	GrB_Index left = 0 ;
	GrB_Index right = A->nvec-1 ;
	const GrB_Index *__restrict__ Ah = (GrB_Index *)A->h ;

	GB_BINARY_SEARCH(i, Ah, left, right, found) ;

	*result = left;
	return found;
}

GrB_Info GxB_MatrixTupleIter_iterate_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_iterate_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;

	if(rowIdx < 0 || rowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				"Row index " GBu " out of range ; must be < " GBu,
				rowIdx, iter->nrows) ;
	}

	if(iter->nvals == 0) {
		// empty matrix
		return (GrB_SUCCESS) ;
	}

	// deplete iterator, should caller ignore returned error
	_EmptyIterator(iter) ;

	GrB_Index _rowIdx = rowIdx ;
	GrB_Matrix A = iter->A ;

	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		if(!_find_row_index_in_Ah(iter->A, rowIdx, &_rowIdx)) {
			// empty row
			return (GrB_SUCCESS) ;
		}
	}

	// init iterator to scan row
	iter->p        =  0 ;
	iter->nvals    =  iter->A->p[_rowIdx + 1] ;
	iter->nnz_idx  =  iter->A->p[_rowIdx] ;
	iter->row_idx  =  _rowIdx ;

	return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_jump_to_row
(
	GxB_MatrixTupleIter *iter,
	GrB_Index rowIdx
) {
	GB_WHERE1("GxB_MatrixTupleIter_jump_to_row (iter, rowIdx)") ;
	GB_RETURN_IF_NULL(iter) ;

	if(rowIdx < 0 || rowIdx >= iter->nrows) {
		GB_ERROR(GrB_INVALID_INDEX,
				"Row index " GBu " out of range ; must be < " GBu,
				rowIdx, iter->nrows) ;
	}

	if(iter->nvals == 0) {
		// empty matrix
		return (GrB_SUCCESS) ;
	}

	// deplete iterator, should caller ignore returned error
	_EmptyIterator(iter) ;

	// this call needed because we deplete the iterator
	GrB_Matrix A = iter->A ;
	GrB_Index _rowIdx = rowIdx ; // row position in A->p

	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		if(!_find_row_index_in_Ah(A, rowIdx, &_rowIdx)) { // In hypersparse _rowIdx should be the index to Ah
			GB_ERROR (GrB_INVALID_INDEX,
				"Row index " GBu " doesn't exist in the hypersparse matrix, row might be empty",
				rowIdx) ;
		}
	}

	iter->p        =  0 ;
	iter->nvals    =  iter->A->p[A->nvec] ;
	iter->nnz_idx  =  iter->A->p[_rowIdx] ;
	iter->row_idx  =  _rowIdx ;

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

	// deplete iterator, should caller ignore returned error
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

	// make sure endRowIdx is within bounds
	GrB_Index nrows;
	GrB_Matrix_nrows(&nrows, iter->A);
	endRowIdx = (endRowIdx < nrows) ? endRowIdx : nrows - 1 ;

	GrB_Index  _endRowIdx    =  endRowIdx    ;
	GrB_Index  _startRowIdx  =  startRowIdx  ;

	if(iter->sparsity_type == GxB_HYPERSPARSE) {
		if(!_find_minimal_row_in_Ah_greater_or_equal_to_rowIdx(iter->A,
					startRowIdx, &_startRowIdx)) {
			return (GrB_SUCCESS) ;
		}

		if(!_find_maximal_row_in_Ah_smaller_or_equal_to_rowIdx(iter->A,
					endRowIdx, &_endRowIdx)) {
			return (GrB_SUCCESS) ;
		}

		// it is possible for _startRowIdx to be greater than _endRowIdx
		// consider the sparse array: [10,20,30]
		// startRowIdx = 15
		// endRowIDx   = 16
		// in this case _startRowIdx = 20 and _endRowIdx = 10
		if(_startRowIdx > _endRowIdx) return (GrB_SUCCESS) ;
	}

	iter->p       =  0 ;
	iter->nvals   = iter->A->p[_endRowIdx + 1] ;
	iter->row_idx = _startRowIdx ;
	iter->nnz_idx = iter->A->p[_startRowIdx] ;

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

	if(col) {
		*col = A->i[nnz_idx] ;
	}
	if(val) {
		GrB_Index offset = nnz_idx * iter->size;
		memcpy(val, (char*)A->x + offset, iter->size);
	}

	//--------------------------------------------------------------------------
	// extract the row indices
	//--------------------------------------------------------------------------

	GrB_Index i = iter->row_idx ;
	GrB_Index nrows = A->nvec ;
	const GrB_Index *__restrict__ Ap = (GrB_Index*)A->p ;
	const GrB_Index *__restrict__ Ah = (GrB_Index*)A->h ;

	for( ; i < nrows ; i++) {
		GrB_Index p = iter->p + Ap[i] ;
		// the number of columns in a row equals to Ap[i+1] - Ap[i] 
		// thus if p == Ap[i+1] means we exhausted the current row.
		if(p < Ap[i + 1]) {
			iter->p++ ;
			break ;
		}
		iter->p = 0 ;
	}

	if(row) *row = (iter->sparsity_type == GxB_SPARSE) ? i : Ah[i] ;

	// update the current row_idx in the iterator
	iter->row_idx = i ;
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
	
	return _init(iter, iter->A) ;
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

	return _init(iter, A) ;
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


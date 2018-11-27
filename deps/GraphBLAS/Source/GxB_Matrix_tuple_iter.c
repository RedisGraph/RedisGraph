//------------------------------------------------------------------------------
// GxB_MatrixTupleIter: Iterates over matrix none zero values
//------------------------------------------------------------------------------

#include "GB.h"

// Create a new iterator
GrB_Info GxB_MatrixTupleIter_new
(
    GxB_MatrixTupleIter **iter,     // iterator to create
    GrB_Matrix A                    // matrix to iterate over
)
{
    GB_WHERE ("GxB_MatrixTupleIter_new (A)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    
    GrB_Index ncols ;
    GrB_Matrix_ncols (&ncols, A) ;

    *iter = NULL ;
    GB_MALLOC_MEMORY (*iter, 1, sizeof (GxB_MatrixTupleIter)) ;
    GrB_Matrix_nvals (&((*iter)->nvals), A) ;
    (*iter)->A = A ;
    (*iter)->nnz_idx = 0 ;
    (*iter)->col_idx = 0 ;
    (*iter)->ncols = ncols ;
    (*iter)->p = A->p[0] ;
    return (GrB_SUCCESS) ;
}

// Update iter to scan specific column
GrB_Info GxB_MatrixTupleIter_iterate_column
(
    GxB_MatrixTupleIter *iter,
    GrB_Index colIdx
)
{
    GB_WHERE ("GxB_MatrixTupleIter_iterate_column (iter, colIdx)") ;
    GB_RETURN_IF_NULL (iter) ;
    
    if (colIdx < 0 && colIdx >= iter->ncols) {
        return (GB_ERROR (GrB_INVALID_INDEX, (GB_LOG, "Column index out of range"))) ;
    }

    iter->nvals = iter->A->p[colIdx+1] ;
    iter->nnz_idx = iter->A->p[colIdx] ;
    iter->col_idx = colIdx ;
    iter->p = 0 ;
    return (GrB_SUCCESS) ;
}

// Advance iterator
GrB_Info GxB_MatrixTupleIter_next
(
    GxB_MatrixTupleIter *iter,      // iterator to consume
    GrB_Index *row,                 // optional output row index
    GrB_Index *col,                 // optional output column index
    bool *depleted                  // indicate if iterator depleted
)
{
    GB_WHERE ("GxB_MatrixTupleIter_next (iter, row, col, depleted)") ;
    GB_RETURN_IF_NULL (iter) ;
    GB_RETURN_IF_NULL (depleted) ;
    GrB_Index nnz_idx = iter->nnz_idx ;

    if (nnz_idx >= iter->nvals) {
        *depleted = true ;
        return (GrB_SUCCESS) ;
    }

    GrB_Matrix A = iter->A ;

    //--------------------------------------------------------------------------
    // extract the row indices
    //--------------------------------------------------------------------------

    if (row) *row = A->i[nnz_idx] ;

    //--------------------------------------------------------------------------
    // extract the column indices
    //--------------------------------------------------------------------------

    const int64_t *Ap = A->p ;
    int64_t j = iter->col_idx ;

    for (; j < iter->ncols ; j++)
    {
        int64_t p = iter->p + Ap [j] ;
        if (p < Ap [j+1]) {
            iter->p++ ;
            if (col) *col = j ;
            break ;
        }
        iter->p = 0 ;
    }
    
    iter->col_idx = j ;
    iter->nnz_idx++ ;

    *depleted = false ;
    return (GrB_SUCCESS) ;
}

// Reset iterator
GrB_Info GxB_MatrixTupleIter_reset
(
    GxB_MatrixTupleIter *iter       // iterator to reset
)
{
    GB_WHERE ("GxB_MatrixTupleIter_reset (iter)") ;
    GB_RETURN_IF_NULL (iter) ;
    iter->nnz_idx = 0 ;
    iter->col_idx = 0 ;
    iter->p = iter->A->p[0] ;
    return (GrB_SUCCESS) ;
}

// Update iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
    GxB_MatrixTupleIter *iter,      // iterator to update
    GrB_Matrix A                    // matrix to scan
)
{
    GB_WHERE ("GxB_MatrixTupleIter_reuse (iter, A)") ;
    GB_RETURN_IF_NULL (iter) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    
    GrB_Index ncols ;
    GrB_Matrix_ncols (&ncols, A) ;

    iter->A = A ;
    iter->ncols = ncols ;
    GrB_Matrix_nvals (&iter->nvals, A) ;
    GxB_MatrixTupleIter_reset (iter) ;
    return (GrB_SUCCESS) ;
}

// Release iterator
GrB_Info GxB_MatrixTupleIter_free
(
    GxB_MatrixTupleIter *iter       // iterator to free
)
{
    GB_WHERE ("GxB_MatrixTupleIter_free (iter)") ;
    GB_RETURN_IF_NULL (iter) ;
    GB_FREE_MEMORY (iter, 1, sizeof (struct GxB_MatrixTupleIter)) ;
    return (GrB_SUCCESS) ;
}

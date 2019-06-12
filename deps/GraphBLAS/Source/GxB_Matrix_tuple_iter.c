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

    GrB_Index n;
    GrB_Matrix_nrows(&n, A);

    *iter = NULL ;
    GB_MALLOC_MEMORY (*iter, 1, sizeof (GxB_MatrixTupleIter)) ;
    GrB_Matrix_nvals (&((*iter)->nvals), A) ;
    (*iter)->A = A ;
    (*iter)->nnz_idx = 0 ;
    (*iter)->idx = 0 ;
    (*iter)->n = n;
    (*iter)->p = A->p[0] ;
    return (GrB_SUCCESS) ;
}

GrB_Info GxB_MatrixTupleIter_iterate
(
    GxB_MatrixTupleIter *iter,
    GrB_Index idx
)
{
    GB_WHERE("GxB_MatrixTupleIter_iterate (iter, idx)");
    GB_RETURN_IF_NULL(iter);

    if (idx < 0 && idx >= iter->n)
    {
        return (GB_ERROR(GrB_INVALID_INDEX, (GB_LOG, "Index out of range")));
    }

    iter->nvals = iter->A->p[idx+ 1];
    iter->nnz_idx = iter->A->p[idx];
    iter->idx = idx;
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
    // extract the column indices
    //--------------------------------------------------------------------------

    if (col)
        *col = A->i[nnz_idx];

    //--------------------------------------------------------------------------
    // extract the row indices
    //--------------------------------------------------------------------------

    const int64_t *Ap = A->p;
    int64_t i = iter->idx;

    for (; i < iter->n; i++)
    {
        int64_t p = iter->p + Ap[i];
        if (p < Ap[i + 1])
        {
            iter->p++;
            if (row)
                *row = i;
            break;
        }
        iter->p = 0;
    }

    iter->idx = i;

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
    iter->idx = 0 ;
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

    GrB_Index n;
    GrB_Matrix_nrows(&n, A);

    iter->A = A ;
    iter->n = n ;
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
    GB_FREE_MEMORY (iter, 1, sizeof (GxB_MatrixTupleIter)) ;
    return (GrB_SUCCESS) ;
}

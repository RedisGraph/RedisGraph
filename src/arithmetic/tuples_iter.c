/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "tuples_iter.h"
#include <assert.h>

TuplesIter *TuplesIter_new
(
    GrB_Matrix A
)
{
    TuplesIter *iter = malloc (sizeof (TuplesIter)) ;
    GrB_Matrix_nvals (&iter->nvals, A) ;
    iter->A = A ;
    iter->nnz_idx = 0 ;
    iter->col_idx = 0 ;
    iter->p = A->p[0] ;
    return iter ;
}

TuplesIter *TuplesIter_iterate_column
(
    TuplesIter *iter,
    GrB_Index colIdx
)
{
    assert(iter) ;
    iter->nvals = iter->A->p[colIdx+1] ;
    iter->nnz_idx = iter->A->p[colIdx] ;
    iter->col_idx = colIdx ;
    iter->p = 0 ;
    return iter ;
}

TuplesIter_Info TuplesIter_next
(
    TuplesIter *iter,
    GrB_Index *row,
    GrB_Index *col
)
{
    GrB_Index nnz_idx = iter->nnz_idx ;

    if (nnz_idx >= iter->nvals) {
        return TuplesIter_DEPLETED ;
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

    for (; j < A->ncols ; j++)
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

    return TuplesIter_OK;
}

void TuplesIter_reset
(
    TuplesIter *iter
)
{
    assert (iter) ;
    iter->nnz_idx = 0 ;
    iter->col_idx = 0 ;
    iter->p = iter->A->p[0] ;
}

void TuplesIter_reuse
(
    TuplesIter *iter,
    GrB_Matrix A
)
{
    assert (iter) ;
    iter->A = A ;
    GrB_Matrix_nvals (&iter->nvals, A) ;
    TuplesIter_reset(iter) ;
}

void TuplesIter_free
(
    TuplesIter *iter
)
{
    free (iter) ;
}

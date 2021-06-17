//------------------------------------------------------------------------------
// GB_export: export a matrix or vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// No conversion is done, and the matrix is exported in its current sparsity
// structure and by-row/by-col format.

#include "GB_export.h"

GrB_Info GB_export      // export a matrix in any format
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix to export
    GrB_Index *vlen,    // vector length
    GrB_Index *vdim,    // vector dimension

    // the 5 arrays:
    GrB_Index **Ap,     // pointers, size nvec+1 for hyper, vdim+1 for sparse
    GrB_Index *Ap_size, // size of Ap

    GrB_Index **Ah,     // vector indices, size nvec for hyper
    GrB_Index *Ah_size, // size of Ah

    int8_t **Ab,        // bitmap, size nzmax
    GrB_Index *Ab_size, // size of Ab

    GrB_Index **Ai,     // indices, size nzmax
    GrB_Index *Ai_size, // size of Ai

    void **Ax,          // values, size nzmax
    GrB_Index *Ax_size, // size of Ax (# of entries)

    // additional information for specific formats:
    GrB_Index *nvals,   // # of entries for bitmap format.
    bool *jumbled,      // if true, sparse/hypersparse may be jumbled.
    GrB_Index *nvec,    // size of Ah for hypersparse format.

    // information for all formats:
    int *sparsity,      // hypersparse, sparse, bitmap, or full
    bool *is_csc,       // if true then matrix is by-column, else by-row
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*A) ;
    ASSERT_MATRIX_OK (*A, "A to export", GB0) ;
    ASSERT (!GB_ZOMBIES (*A)) ;
    ASSERT (GB_JUMBLED_OK (*A)) ;
    ASSERT (!GB_PENDING (*A)) ;

    GB_RETURN_IF_NULL (type) ;
    GB_RETURN_IF_NULL (vlen) ;
    GB_RETURN_IF_NULL (vdim) ;
    GB_RETURN_IF_NULL (Ax) ;
    GB_RETURN_IF_NULL (Ax_size) ;

    int s = GB_sparsity (*A) ;
    switch (s)
    {
        case GxB_HYPERSPARSE : 
            GB_RETURN_IF_NULL (nvec) ;
            GB_RETURN_IF_NULL (Ah) ; GB_RETURN_IF_NULL (Ah_size) ;

        case GxB_SPARSE : 
            GB_RETURN_IF_NULL (Ap) ; GB_RETURN_IF_NULL (Ap_size) ;
            GB_RETURN_IF_NULL (Ai) ; GB_RETURN_IF_NULL (Ai_size) ;
            break ;

        case GxB_BITMAP : 
            GB_RETURN_IF_NULL (nvals) ;
            GB_RETURN_IF_NULL (Ab) ; GB_RETURN_IF_NULL (Ab_size) ;

        case GxB_FULL : 
            break ;

        default: ;
    }

    //--------------------------------------------------------------------------
    // export the matrix
    //--------------------------------------------------------------------------

    (*type) = (*A)->type ;
    (*vlen) = (*A)->vlen ;
    (*vdim) = (*A)->vdim ;
    (*Ax) = (*A)->x ; (*A)->x = NULL ; (*Ax_size) = (*A)->nzmax ;

    switch (s)
    {
        case GxB_HYPERSPARSE : 
            (*nvec) = (*A)->nvec ;
            (*Ah) = (*A)->h ; (*A)->h = NULL ; (*Ah_size) = (*A)->plen ;

        case GxB_SPARSE : 
            if (jumbled != NULL)
            { 
                (*jumbled) = (*A)->jumbled ;
            }
            (*Ap) = (*A)->p ; (*A)->p = NULL ; (*Ap_size) = (*A)->plen + 1 ;
            (*Ai) = (*A)->i ; (*A)->i = NULL ; (*Ai_size) = (*A)->nzmax ;
            break ;

        case GxB_BITMAP : 
            (*nvals) = (*A)->nvals ;
            (*Ab) = (*A)->b ; (*A)->b = NULL ; (*Ab_size) = (*A)->nzmax ;

        case GxB_FULL : 

        default: ;
    }

    if (sparsity != NULL)
    { 
        (*sparsity) = s ;
    }
    if (is_csc != NULL)
    { 
        (*is_csc) = (*A)->is_csc ;
    }

    GB_Matrix_free (A) ;
    ASSERT ((*A) == NULL) ;
    return (GrB_SUCCESS) ;
}


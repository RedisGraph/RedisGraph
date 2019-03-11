//------------------------------------------------------------------------------
// GB_AxB_alloc: estimate nnz(C) and allocate C for C=A*B or C=A'*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// parallel: this function will remain sequential.
// parallelism will be done in GB_AxB_parallel.

// Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.

#include "GB.h"

GrB_Info GB_AxB_alloc           // estimate nnz(C) and allocate C for C=A*B
(
    GrB_Matrix *Chandle,        // output matrix
    const GrB_Type ctype,       // type of C
    const GrB_Index cvlen,      // vector length of C
    const GrB_Index cvdim,      // # of vectors of C
    const GrB_Matrix M,         // optional mask
    const GrB_Matrix A,         // input matrix A (transposed for dot product)
    const GrB_Matrix B,         // input matrix B
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    const int64_t rough_guess   // rough estimate of nnz(C)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_Context Context = NULL ;
    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for alloc C=A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for alloc C=A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for alloc C=A*B", GB0)) ;

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // determine hypersparsity
    //--------------------------------------------------------------------------

    // C is hypersparse if any of A, B, and/or M are hypersparse
    bool C_is_hyper = (cvdim > 1) &&
        (A->is_hyper || B->is_hyper || (M != NULL && M->is_hyper)) ;

    //--------------------------------------------------------------------------
    // estimate nnz(C)
    //--------------------------------------------------------------------------

    int64_t cnz_guess ;

    if (M == NULL)
    { 
        // estimate the number of nonzeros for C=A*B (or C=A'*B for dot
        // product method)

        cnz_guess = rough_guess ;

        // abnzmax = cvlen * cvdim, but check for overflow
        GrB_Index abnzmax ;
        if (GB_Index_multiply (&abnzmax, cvlen, cvdim))
        {
            // only do this if cvlen * cvdim does not overflow
            cnz_guess = GB_IMIN (cnz_guess, abnzmax) ;
        }
    }
    else
    { 
        // the pattern of C is a subset of the mask
        cnz_guess = GB_NNZ (M) ;
    }

    // add one to ensure cnz_guess > 0, and (cnz < C->nzmax) will always hold
    // if cnz_guess is exact.
    cnz_guess++ ;

    //--------------------------------------------------------------------------
    // allocate C
    //--------------------------------------------------------------------------

    // Use CSC format but this is ignored for now.  If hypersparse, assume C or
    // as many non-empty columns of B (compute it if it has not already been
    // computed).  This is an upper bound.  If nnz(B(:,j)) is zero, this
    // implies nnz(C(:,j)) will be zero, but not the other way around.  That
    // is, nnz(B(:,j)) can be > 0, but if nnz(A(k,j)) == 0 for all k for
    // entries B(k,j), then nnz(C(:,j)) will be zero.

    // C->p and C->h are allocated but not initialized.

    int64_t cplen = -1 ;

    if (C_is_hyper)
    {
        if (B->nvec_nonempty < 0)
        { 
            B->nvec_nonempty = GB_nvec_nonempty (B, NULL) ;
        }
        cplen = B->nvec_nonempty ;
    }

    GB_CREATE (Chandle, ctype, cvlen, cvdim, GB_Ap_malloc, true,
        GB_SAME_HYPER_AS (C_is_hyper), B->hyper_ratio, cplen,
        cnz_guess, numeric, NULL) ;

    return (info) ;
}


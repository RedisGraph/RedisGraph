//------------------------------------------------------------------------------
// GB_AxB_alloc: estimate nnz(C) and allocate C for C=A*B or C=A'*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

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
    const int64_t rough_guess,  // rough estimate of nnz(C)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

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

    // Use CSC format but this is ignored for now.  If hypersparse, assume C
    // has as many non-empty columns as B.  C->p and C->h are allocated but not
    // initialized.

    GB_CREATE (Chandle, ctype, cvlen, cvdim, GB_Ap_malloc, true,
        GB_SAME_HYPER_AS (C_is_hyper), B->hyper_ratio, B->nvec_nonempty,
        cnz_guess, numeric) ;

    return (info) ;
}


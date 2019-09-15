//------------------------------------------------------------------------------
// GB_AxB_alloc: estimate nnz(C) and allocate C for C=A*B or C<M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.
// This used for Gustavon's method and the heap-based method, not the dot
// product method.

#include "GB_mxm.h"
#include "GB_iterator.h"

GrB_Info GB_AxB_alloc           // estimate nnz(C) and allocate C for C=A*B
(
    GrB_Matrix *Chandle,        // output matrix
    const GrB_Type ctype,       // type of C
    const GrB_Index cvlen,      // vector length of C
    const GrB_Index cvdim,      // # of vectors of C
    const GrB_Matrix M,         // optional mask
    const GrB_Matrix A,         // input matrix A
    const GrB_Matrix B,         // input matrix B
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    const int64_t cnz_extra     // added to the rough estimate (if M NULL)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    GB_Context Context = NULL ;
    #endif
    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for alloc C=A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for alloc C=A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for alloc C=A*B", GB0)) ;
    ASSERT (M == NULL || !M->is_slice) ;

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // determine hypersparsity
    //--------------------------------------------------------------------------

    // C is hypersparse if any of A, B, and/or M are hypersparse
    // or if A or B are slice or hyperslice.  M is never a slice or hyperslice.
    bool C_is_hyper = (cvdim > 1) &&
        (A->is_hyper || B->is_hyper || (M != NULL && M->is_hyper)) ;
    C_is_hyper = C_is_hyper || (A->is_slice || B->is_slice) ;

    //--------------------------------------------------------------------------
    // estimate nnz(C) and the # of vectors in C
    //--------------------------------------------------------------------------

    int64_t cplen = 0 ;
    int64_t cnz_guess = 0 ;

    if (M != NULL)
    {

        //----------------------------------------------------------------------
        // C<M>=A*B, pattern of C is a subset of M
        //----------------------------------------------------------------------

        // cnz_guess is a strict upper bound on nnz(C)

        const int64_t *restrict Mp = M->p ;
        const int64_t *restrict Mh = M->h ;
        const int64_t mnvec = M->nvec ;
        int64_t mpleft = 0 ;
        int64_t mpright = mnvec - 1 ;
        const bool M_is_hyper = M->is_hyper ;
        int64_t bnvec_nonempty = 0 ;

        GBI_for_each_vector (B)
        { 

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            GBI_jth_iteration (j, pB, pB_end) ;
            int64_t bjnz = pB_end - pB ;
            if (bjnz == 0) continue ;
            bnvec_nonempty++ ;

            //------------------------------------------------------------------
            // get M(:,j)
            //------------------------------------------------------------------

            // find vector j in M
            int64_t pM_start, pM_end ;
            GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright, j, &pM_start,
                &pM_end) ;
            int64_t mjnz = pM_end - pM_start ;
            cnz_guess += mjnz ;

            if (mjnz != 0) cplen++ ;
        }

        if (B->nvec_nonempty < 0) B->nvec_nonempty = bnvec_nonempty ;
        ASSERT (B->nvec_nonempty == GB_nvec_nonempty (B, NULL)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C=A*B
        //----------------------------------------------------------------------

        // cnz_guess is a rough guess, just for allocating C->i for the
        // symbolic phase for Gustavson, or during the combined symbolic/
        // numeric phase for the heap-based method.  In either case, this space
        // is reallocated if cnz_guess is too low, via GB_ix_realloc.

        cnz_guess = cnz_extra + GB_NNZ (A) + GB_NNZ (B) ;

        // abnzmax = cvlen * cvdim, but check for overflow
        GrB_Index abnzmax ;
        if (GB_Index_multiply (&abnzmax, cvlen, cvdim))
        { 
            // only do this if cvlen * cvdim does not overflow
            cnz_guess = GB_IMIN (cnz_guess, abnzmax) ;
        }

        if (C_is_hyper)
        {
            if (B->nvec_nonempty < 0)
            {
                B->nvec_nonempty = GB_nvec_nonempty (B, NULL) ;
            }
            cplen = B->nvec_nonempty ;
        }
    }

    //--------------------------------------------------------------------------
    // allocate C
    //--------------------------------------------------------------------------

    // C->p and C->h are allocated but not initialized.  C->i is allocated.
    // C->x is allocated if the numeric flag is true.

    // add one to ensure cnz_guess > 0, and (cnz < C->nzmax) will always hold
    // if cnz_guess is exact.

    GB_CREATE (Chandle, ctype, cvlen, cvdim, GB_Ap_malloc, true,
        GB_SAME_HYPER_AS (C_is_hyper), B->hyper_ratio, cplen,
        cnz_guess + 1, numeric, NULL) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (info) ;
}


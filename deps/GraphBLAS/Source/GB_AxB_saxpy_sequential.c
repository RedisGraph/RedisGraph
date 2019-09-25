//------------------------------------------------------------------------------
// GB_AxB_saxpy_sequential: C<M>=A*B using a single thread
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Sequential matrix-matrix multiply using a saxpy-based method.  See
// GB_AxB_parallel for a description of the arguments.  Only a single thread
// does this work.

// Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.

#include "GB_mxm.h"

GrB_Info GB_AxB_saxpy_sequential    // single-threaded C<M>=A*B
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// already chosen
    const int64_t bjnz_max,         // for heap method only
    const bool check_for_dense_mask,// if true, check floplimit for mask 
    bool *mask_applied,             // if true, mask was applied
    const int Sauna_id              // Sauna to use, for Gustavson method only
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    // only one thread does this entire function
    GB_Context Context = NULL ;
    #endif
    ASSERT (Chandle != NULL) ;          // C = (*Chandle) is NULL
    ASSERT (*Chandle == NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for sequential A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for sequential A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for sequential A*B", GB0)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for sequential A*B", GB0)) ;
    ASSERT (mask_applied != NULL) ;
    ASSERT (AxB_method != GxB_AxB_DOT) ;

    //----------------------------------------------------------------------
    // C<M> = A*B via a saxpy-based method
    //----------------------------------------------------------------------

    // Decide whether or not to use the mask.

    if (M != NULL)
    {
        if (Mask_comp)
        { 
            // the saxpy methods cannot handle a complemented mask at all.
            // Discard the mask; mask_applied will be false.
            M = NULL ;
        }
        else if (check_for_dense_mask)
        {
            // The saxpy methods can handle any mask that's not complemented,
            // but they will examine each entry in the mask.  This is costly if
            // (total_flops < nnz(M)).  This condition is not checked if it has
            // already been considered in the caller.
            int64_t floplimit = GB_NNZ (M) ;
            if (GB_AxB_flopcount (NULL, NULL, M, A, B, floplimit, NULL))
            { 
                // total_flops < nnz(M), so the mask is too dense to use.
                // Discard the mask; mask_applied will be false.
                M = NULL ;
            }
        }
    }

    // do the matrix multiply
    if (AxB_method == GxB_AxB_HEAP)
    { 
        // C<M> = A*B via heap method
        ASSERT (A->vdim == B->vlen) ;
        return (GB_AxB_heap (Chandle, M, Mask_comp, A, B, semiring,
            flipxy, mask_applied, bjnz_max)) ;
    }
    else // AxB_method == GxB_AxB_GUSTAVSON
    { 
        // C<M> = A*B via Gustavson method
        ASSERT (A->vdim == B->vlen) ;
        return (GB_AxB_Gustavson (Chandle, M, Mask_comp, A, B, semiring,
            flipxy, mask_applied, Sauna_id)) ;
    }
}


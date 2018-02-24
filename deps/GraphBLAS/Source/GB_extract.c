//------------------------------------------------------------------------------
// GB_extract: C = A (I,J) with Mask, descriptor, and accum
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Not user-callable.  Implements the user-callable GrB_*_extract functions.

#include "GB.h"

GrB_Info GB_extract                 // C<Mask> = accum (C, A(I,J))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C matrix descriptor
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,           // Mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // A matrix descriptor
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Index *J,             // column indices
    const GrB_Index nj              // number of column indices
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C, Mask, and A checked in caller, GrB_*_extract
    RETURN_IF_NULL (I) ;            // I = GrB_ALL indicates A(:,J)
    RETURN_IF_NULL (J) ;            // J = GrB_ALL indicates A(I,:)
    RETURN_IF_UNINITIALIZED (accum) ;

    ASSERT_OK (GB_check (C, "C input for GB_Matrix_extract", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_Matrix_extract", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_Matrix_extract", 0)) ;
    ASSERT_OK (GB_check (A, "A input for GB_Matrix_extract", 0)) ;

    // check domains and dimensions for C<Mask> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, Mask, accum, A->type) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // check the dimensions; this does no work at all, just checks dimensions
    info = GB_subref_numeric (NULL, C->nrows, C->ncols, A, I, ni, J, nj,
        A_transpose) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // quick return if an empty Mask is complemented
    RETURN_IF_QUICK_MASK (C, C_replace, Mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (C) ;
    APPLY_PENDING_UPDATES (Mask) ;
    APPLY_PENDING_UPDATES (A) ;

    //--------------------------------------------------------------------------
    // T = A (I,J) or (A (J,I))' where T has the same type as A
    //--------------------------------------------------------------------------

    GrB_Matrix T ;
    info = GB_subref_numeric (&T, 0, 0, A, I, ni, J, nj, A_transpose) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
}


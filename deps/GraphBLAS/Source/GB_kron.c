//------------------------------------------------------------------------------
// GB_kron: C<Mask> = accum (C, kron(A,B))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C, kron(A,B))

// The input matrices A and B are optionally transposed.

// Not user-callable.  Does the work for GxB_kron

#include "GB.h"

GrB_Info GB_kron                    // C<Mask> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use ~Mask
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '*' for kron(A,B)
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose          // if true, use B' instead of B
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (B) ;
    RETURN_IF_UNINITIALIZED (Mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (op) ;
    RETURN_IF_UNINITIALIZED (accum) ;

    ASSERT_OK (GB_check (C, "C input for GB_kron", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_kron", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_kron", 0)) ;
    ASSERT_OK (GB_check (op, "op for GB_kron", 0)) ;
    ASSERT_OK (GB_check (A, "A for GB_kron", 0)) ;
    ASSERT_OK (GB_check (B, "B for GB_kron", 0)) ;

    // check domains and dimensions for C<Mask> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, Mask, accum, op->ztype) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // T=op(A,B) via op operator, so A and B must be compatible with z=op(a,b)
    info = GB_BinaryOp_compatible (op, NULL, A->type, B->type, 0) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // check the dimensions of C
    int64_t anrows = (A_transpose) ? A->ncols : A->nrows ;
    int64_t ancols = (A_transpose) ? A->nrows : A->ncols ;
    int64_t bnrows = (B_transpose) ? B->ncols : B->nrows ;
    int64_t bncols = (B_transpose) ? B->nrows : B->ncols ;
    GrB_Index cnrows, cncols, cnz = 0 ;
    bool ok = GB_Index_multiply (&cnrows, anrows,  bnrows) ;
    ok = ok && GB_Index_multiply (&cncols, ancols,  bncols) ;
    ok = ok && GB_Index_multiply (&cnz, NNZ (A), NNZ (B)) ;
    if (!ok || C->nrows != cnrows || C->ncols != cncols)
    {
        return (ERROR (GrB_DIMENSION_MISMATCH, (LOG, "%s:\n"
            "output is "GBd"-by-"GBd"; must be "GBd"-by-"GBd"\n"
            "first input is "GBd"-by-"GBd"%s with "GBd" entries\n"
            "second input is "GBd"-by-"GBd"%s with "GBd" entries",
            ok ? "Dimensions not compatible:" : "Problem too large:",
            C->nrows, C->ncols, cnrows, cncols,
            anrows, ancols, A_transpose ? " (transposed)" : "", NNZ (A),
            bnrows, bncols, B_transpose ? " (transposed)" : "", NNZ (B)))) ;
    }

    // quick return if an empty Mask is complemented
    RETURN_IF_QUICK_MASK (C, C_replace, Mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (C) ;
    APPLY_PENDING_UPDATES (Mask) ;
    APPLY_PENDING_UPDATES (A) ;
    APPLY_PENDING_UPDATES (B) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix T
    //--------------------------------------------------------------------------

    // T has the same type as z for the multiply operator, z=op(x,y)
    GrB_Matrix T ;
    GB_NEW (&T, op->ztype, C->nrows, C->ncols, true, false) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    double memory = 0 ;
    if (!GB_Matrix_alloc (T, cnz, true, &memory))
    {
        // out of memory
        GB_Matrix_clear (C) ;           // C is now initialized, just empty
        GB_MATRIX_FREE (&T) ;
        ASSERT_OK (GB_check (C, "C cleared", 0)) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    //--------------------------------------------------------------------------
    // transpose A and B if requested
    //--------------------------------------------------------------------------

    GrB_Matrix AT = NULL ;
    if (A_transpose)
    {
        // AT = A' and typecast to op->xtype
        GB_NEW (&AT, op->xtype, anrows, ancols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&T) ;
            return (info) ;
        }
        info = GB_Matrix_transpose (AT, A, NULL, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            GB_MATRIX_FREE (&T) ;
            return (info) ;
        }
    }

    GrB_Matrix BT = NULL ;
    if (B_transpose)
    {
        // BT = B' and typecast to op->ytype
        GB_NEW (&BT, op->ytype, bnrows, bncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            GB_MATRIX_FREE (&T) ;
            return (info) ;
        }
        info = GB_Matrix_transpose (BT, B, NULL, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            GB_MATRIX_FREE (&BT) ;
            GB_MATRIX_FREE (&T) ;
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // T = kron(A,B)
    //--------------------------------------------------------------------------

    GB_kron_kernel (T, op, A_transpose ? AT : A, B_transpose ? BT : B) ;

    // free workspace
    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&BT) ;

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
}


//------------------------------------------------------------------------------
// GB_mxm: matrix-matrix multiply for GrB_mxm, GrB_mxv, and GrB_vxm
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C,A*B) and variations.

// For GrB_mxv and GrB_vxm, the B input is always n-by-1 and it is never
// transposed.

// This function is not user-callable.  It does the work for user-callable
// functions GrB_mxm, GrB_mxv, and GrB_vxm.

#include "GB.h"

GrB_Info GB_mxm                     // C<Mask> = A*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use ~Mask
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for C=A*B
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose,         // if true, use B' instead of B
    const bool flipxy               // if true, do z=fmult(b,a) vs fmult(a,b)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C, Mask, A, and B checked in GrB_mxm, GrB_mxv, or GrB_vxm
    RETURN_IF_UNINITIALIZED (accum) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (semiring) ;

    ASSERT_OK (GB_check (C, "C input for GB_mxm", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_mxm", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_mxm", 0)) ;
    ASSERT_OK (GB_check (semiring, "semiring for GB_mxm", 0)) ;
    ASSERT_OK (GB_check (A, "A for GB_mxm", 0)) ;
    ASSERT_OK (GB_check (B, "B for GB_mxm", 0)) ;

    // check domains and dimensions for C<Mask> = accum (C,T)
    GrB_Type T_type = semiring->add->op->ztype ;
    GrB_Info info = GB_compatible (C->type, C, Mask, accum, T_type) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // T=A*B via semiring, so A and B must be compatible with semiring->mult
    if (flipxy)
    {
        // z=fmult(b,a), for entries a from A, and b from B
        info = GB_BinaryOp_compatible (semiring->multiply,
                                        NULL, B->type, A->type, 0) ;
    }
    else
    {
        // z=fmult(a,b), for entries a from A, and b from B
        info = GB_BinaryOp_compatible (semiring->multiply,
                                        NULL, A->type, B->type, 0) ;
    }
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // check the dimensions
    int64_t anrows = (A_transpose) ? A->ncols : A->nrows ;
    int64_t ancols = (A_transpose) ? A->nrows : A->ncols ;
    int64_t bnrows = (B_transpose) ? B->ncols : B->nrows ;
    int64_t bncols = (B_transpose) ? B->nrows : B->ncols ;
    if (ancols != bnrows || C->nrows != anrows || C->ncols != bncols)
    {
        return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "first input is "GBd"-by-"GBd"%s\n"
            "second input is "GBd"-by-"GBd"%s",
            C->nrows, C->ncols,
            anrows, ancols, A_transpose ? " (transposed)" : "",
            bnrows, bncols, B_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty Mask is complemented
    RETURN_IF_QUICK_MASK (C, C_replace, Mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (C) ;
    APPLY_PENDING_UPDATES (Mask) ;
    APPLY_PENDING_UPDATES (A) ;
    APPLY_PENDING_UPDATES (B) ;

    //--------------------------------------------------------------------------
    // T = A*B, A'*B, A*B', or A'*B', also using the Mask to cut time and memory
    //--------------------------------------------------------------------------

    // Note that GrB_mxv and GrB_vxm use only A*B and A'*B

    GrB_Matrix T ;
    GB_NEW (&T, T_type, C->nrows, C->ncols, false, true) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // the mask is used to cut time and memory usage for GB_Matrix_multiply,
    // but only if it is not complemented.
    bool mask_applied = false ;
    info = GB_Matrix_multiply (T, ((Mask_comp) ? NULL : Mask), A, B, semiring,
        A_transpose, B_transpose, flipxy, &mask_applied) ;
    if (info != GrB_SUCCESS)
    {
        GB_MATRIX_FREE (&T) ;
        return (info) ;
    }

    ASSERT_OK (GB_check (T, "T=A*B from GB_Matrix_multiply", 0)) ;

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    if ((accum == NULL)
        && (Mask == NULL || (Mask != NULL && mask_applied))
        && (C_replace || NNZ (C) == 0))
    {
        // C = 0 ; C = (ctype) T
        // The Mask (if any) has already been applied in GB_Matrix_multiply.
        // If C is also empty, or to be cleared anyway, and if accum is not
        // present, then T can be transplanted directly into C, as C = (ctype)
        // T, typecasting if needed.  If no typecasting is done then this takes
        // no time at all and is a pure transplant.  If T has zombies then they
        // are safely transplanted into C, and are left in the final result, C.
        ASSERT (ZOMBIES_OK (T)) ;
        return (GB_Matrix_transplant (C, C->type, &T)) ;
    }
    else
    {
        // C<Mask> = accum (C,T)
        // T may have zombies from the masked multiply, so delete them now.
        ASSERT (ZOMBIES_OK (T)) ;
        APPLY_PENDING_UPDATES (T) ;
        ASSERT (!ZOMBIES (T)) ;
        return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
    }
}


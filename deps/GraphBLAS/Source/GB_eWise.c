//------------------------------------------------------------------------------
// GB_eWise: C<Mask> = accum (C, A+B) or A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C,A+B), A.*B and variations.

// The input matrices A and B are optionally transposed.

// Not user-callable.  Does the work for four user-callable functions:
//      GrB_eWiseAdd_*_BinaryOp
//      GrB_eWiseMult_*_BinaryOp

#include "GB.h"

GrB_Info GB_eWise                   // C<Mask> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use ~Mask
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '+' for C=A+B, or .* for A.*B
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose,         // if true, use B' instead of B
    const bool eWiseAdd             // if true, do set union (like A+B),
                                    // otherwise do intersection (like A.*B)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // all other inputs have already been checked
    RETURN_IF_UNINITIALIZED (accum) ;

    ASSERT_OK (GB_check (C, "C input for GB_eWise", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_eWise", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_eWise", 0)) ;
    ASSERT_OK (GB_check (op, "op for GB_eWise", 0)) ;
    ASSERT_OK (GB_check (A, "A for GB_eWise", 0)) ;
    ASSERT_OK (GB_check (B, "B for GB_eWise", 0)) ;

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

    if (eWiseAdd)
    {
        // C = A is done for entries in A but not C
        if (!GB_Type_compatible (C->type, A->type))
        {
            return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
                "first input of type [%s]\n"
                "cannot be typecast to final output of type [%s]",
                A->type->name, C->type->name))) ;
        }
        // C = B is done for entries in B but not C
        if (!GB_Type_compatible (C->type, B->type))
        {
            return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
                "second input of type [%s]\n"
                "cannot be typecast to final output of type [%s]",
                B->type->name, C->type->name))) ;
        }
    }

    // check the dimensions
    int64_t anrows = (A_transpose) ? A->ncols : A->nrows ;
    int64_t ancols = (A_transpose) ? A->nrows : A->ncols ;
    int64_t bnrows = (B_transpose) ? B->ncols : B->nrows ;
    int64_t bncols = (B_transpose) ? B->nrows : B->ncols ;
    if (anrows != bnrows || ancols != bncols ||
        C->nrows != anrows || C->ncols != bncols)
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
    // T = A+B, A'+B, A+B', or A'+B'
    //--------------------------------------------------------------------------

    // The matrix T is passed as the first input parameter of GB_Matrix_add,
    // GB_Matrix_emult, or GB_Matrix_transpose.  All of them can accept T->p
    // malloc'd, and not initialized, and then each sets T as initialized.
    GrB_Matrix T ;
    GB_NEW (&T, op->ztype, C->nrows, C->ncols, false, true) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    if (A_transpose)
    {
        if (B_transpose)
        {

            //------------------------------------------------------------------
            // T = A'+B'
            //------------------------------------------------------------------

            // [ create S, S->p malloc'd and not initialzed
            GrB_Matrix S ;
            GB_NEW (&S, T->type, A->nrows, A->ncols, false, true) ;
            if (info != GrB_SUCCESS)
            {
                GB_MATRIX_FREE (&T) ;
                return (info) ;
            }

            // S = A + B
            if (eWiseAdd)
            {
                info = GB_Matrix_add (S, A, B, op) ;
            }
            else
            {
                info = GB_Matrix_emult (S, A, B, op) ;
            }
            if (info != GrB_SUCCESS)
            {
                GB_MATRIX_FREE (&T) ;
                GB_MATRIX_FREE (&S) ;
                return (info) ;
            }
            // S->p now initialized ]

            // T = S', no typecasting, no operator
            ASSERT (T->type == S->type) ;
            info = GB_Matrix_transpose (T, S, NULL, true) ;
            GB_MATRIX_FREE (&S) ;

        }
        else
        {

            //------------------------------------------------------------------
            // T = A'+B
            //------------------------------------------------------------------

            // [ create AT, AT->p malloc'ed and not initialized
            GrB_Matrix AT ;
            GB_NEW (&AT, A->type, A->ncols, A->nrows, false, true) ;
            if (info != GrB_SUCCESS)
            {
                GB_MATRIX_FREE (&T) ;
                return (info) ;
            }

            // AT = A', no typecasting, no operator
            ASSERT (AT->type == A->type) ;
            info = GB_Matrix_transpose (AT, A, NULL, true) ;
            if (info != GrB_SUCCESS)
            {
                GB_MATRIX_FREE (&T) ;
                GB_MATRIX_FREE (&AT) ;
                return (info) ;
            }
            // AT->p initialized ]

            // T = AT + B
            if (eWiseAdd)
            {
                info = GB_Matrix_add (T, AT, B, op) ;
            }
            else
            {
                info = GB_Matrix_emult (T, AT, B, op) ;
            }
            GB_MATRIX_FREE (&AT) ;

        }
    }
    else
    {
        if (B_transpose)
        {

            //------------------------------------------------------------------
            // T = A+B'
            //------------------------------------------------------------------

            // [ create BT, BT->p malloc'ed and not initialized
            GrB_Matrix BT ;
            GB_NEW (&BT, B->type, B->ncols, B->nrows, false, true) ;
            if (info != GrB_SUCCESS)
            {
                GB_MATRIX_FREE (&T) ;
                return (info) ;
            }

            // BT = B', no typecasting, no operator
            ASSERT (BT->type == B->type) ;
            info = GB_Matrix_transpose (BT, B, NULL, true) ;
            if (info != GrB_SUCCESS)
            {
                GB_MATRIX_FREE (&T) ;
                GB_MATRIX_FREE (&BT) ;
                return (info) ;
            }
            // BT->p initialized ]

            // T = A + BT
            if (eWiseAdd)
            {
                info = GB_Matrix_add (T, A, BT, op) ;
            }
            else
            {
                info = GB_Matrix_emult (T, A, BT, op) ;
            }
            GB_MATRIX_FREE (&BT) ;

        }
        else
        {

            //------------------------------------------------------------------
            // T = A+B
            //------------------------------------------------------------------

            if (eWiseAdd)
            {
                info = GB_Matrix_add (T, A, B, op) ;
            }
            else
            {
                info = GB_Matrix_emult (T, A, B, op) ;
            }
        }
    }

    if (info != GrB_SUCCESS)
    {
        GB_MATRIX_FREE (&T) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
}


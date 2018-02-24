//------------------------------------------------------------------------------
// GB_apply: apply a unary operator; optionally transpose a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C, op(A)) or op(A)').  This function is not user-callable.
// It does the work for GrB_*_apply.
// Compare this function with GrB_transpose.

#include "GB.h"

GrB_Info GB_apply                   // C<Mask> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,           // Mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Matrix A,             // input matrix
    const bool A_transpose          // A matrix descriptor
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C, Mask, and A already checked in caller
    RETURN_IF_UNINITIALIZED (accum) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (op) ;

    ASSERT_OK (GB_check (C, "C input for GB_apply", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_apply", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_apply", 0)) ;
    ASSERT_OK (GB_check (op, "op for GB_apply", 0)) ;
    ASSERT_OK (GB_check (A, "A input for GB_apply", 0)) ;

    // check domains and dimensions for C<Mask> = accum (C,T)
    GrB_Type T_type = op->ztype ;
    GrB_Info info = GB_compatible (C->type, C, Mask, accum, T_type) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // C = op (A) must be compatible, already checked in GB_compatible
    // A must also be compatible with op->xtype
    if (!GB_Type_compatible (A->type, op->xtype))
    {
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "incompatible type for z=%s(x):\n"
            "input A type [%s]\n"
            "cannot be typecast to operator x of type [%s]",
            op->name, A->type->name, op->xtype->name))) ;
    }

    // check the dimensions
    int64_t tnrows = (A_transpose) ? A->ncols : A->nrows ;
    int64_t tncols = (A_transpose) ? A->nrows : A->ncols ;
    if (C->nrows != tnrows || C->ncols != tncols)
    {
        return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "input is "GBd"-by-"GBd"%s",
            C->nrows, C->ncols,
            tnrows, tncols, A_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty Mask is complemented
    RETURN_IF_QUICK_MASK (C, C_replace, Mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (C) ;
    APPLY_PENDING_UPDATES (Mask) ;
    APPLY_PENDING_UPDATES (A) ;

    //--------------------------------------------------------------------------
    // T = op(A) or op(A')
    //--------------------------------------------------------------------------

    GrB_Matrix T = NULL ;

    if (A_transpose)
    {
        // [ create T; type is op->ztype, malloc T->p
        GB_NEW (&T, T_type, tnrows, tncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }
        info = GB_Matrix_transpose (T, A, op, true) ;
        // T is initialized ]
    }
    else
    {
        // T = op (A), pattern is a shallow copy of A, type is op->ztype.  If
        // op is the built-in IDENTITY and A->type is op->xtype == op->ztype,
        // then a pure shallow copy is made.
        info = GB_shallow_op (&T, op, A) ;
    }

    if (info != GrB_SUCCESS)
    {
        GB_MATRIX_FREE (&T) ;
        return (info) ;
    }

    ASSERT (T->type == op->ztype) ;

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
}


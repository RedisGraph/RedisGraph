//------------------------------------------------------------------------------
// GB_apply: apply a unary operator; optionally transpose a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C, op(A)) or accum (C, op(A)')
// GB_apply does the work for GrB_*_apply.  Compare this with GrB_transpose.

#include "GB.h"

GrB_Info GB_apply                   // C<M> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // M descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    bool A_transpose,               // A matrix descriptor
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK2 (C, M, A)) ;

    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;

    ASSERT_OK (GB_check (C, "C input for GB_apply", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for GB_apply", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_apply", GB0)) ;
    ASSERT_OK (GB_check (op, "op for GB_apply", GB0)) ;
    ASSERT_OK (GB_check (A, "A input for GB_apply", GB0)) ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Type T_type = op->ztype ;
    GrB_Info info = GB_compatible (C->type, C, M, accum, T_type, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // C = op (A) must be compatible, already checked in GB_compatible
    // A must also be compatible with op->xtype
    if (!GB_Type_compatible (A->type, op->xtype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for z=%s(x):\n"
            "input A type [%s]\n"
            "cannot be typecast to operator x of type [%s]",
            op->name, A->type->name, op->xtype->name))) ;
    }

    // check the dimensions
    int64_t tnrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t tncols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    if (GB_NROWS (C) != tnrows || GB_NCOLS (C) != tncols)
    { 
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "input is "GBd"-by-"GBd"%s",
            GB_NROWS (C), GB_NCOLS (C),
            tnrows, tncols, A_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;

    //--------------------------------------------------------------------------
    // T = op(A) or op(A')
    //--------------------------------------------------------------------------

    bool C_is_csc = C->is_csc ;
    if (C_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    GrB_Matrix T = NULL ;

    if (A_transpose)
    { 
        // T = op (A'), typecastint to op->ztype
        // transpose: typecast, apply an op, not in place
        info = GB_transpose (&T, T_type, C_is_csc, A, op, Context) ;
    }
    else
    { 
        // T = op (A), pattern is a shallow copy of A, type is op->ztype.  If
        // op is the built-in IDENTITY and A->type is op->xtype == op->ztype,
        // then a pure shallow copy is made.
        info = GB_shallow_op (&T, C_is_csc, op, A, Context) ;
    }

    if (info != GrB_SUCCESS)
    { 
        GB_MATRIX_FREE (&T) ;
        return (info) ;
    }

    ASSERT (T->is_csc == C->is_csc) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the M
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Context)) ;
}


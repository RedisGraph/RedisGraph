//------------------------------------------------------------------------------
// GB_kron: C<M> = accum (C, kron(A,B))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C<M> = accum (C, kron(A,B))

// The input matrices A and B are optionally transposed.

#include "GB_kron.h"
#include "GB_mxm.h"
#include "GB_transpose.h"
#include "GB_accum_mask.h"

#define GB_FREE_ALL         \
    GB_Matrix_free (&AT) ;  \
    GB_Matrix_free (&BT) ;

GrB_Info GB_kron                    // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op_in,       // defines '*' for kron(A,B)
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M, A, and/or B

    GrB_Info info ;
    GrB_Matrix AT = NULL ;
    GrB_Matrix BT = NULL ;
    GrB_BinaryOp op = op_in ;

    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (B) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;

    ASSERT_MATRIX_OK (C, "C input for GB_kron", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_kron", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_kron", GB0) ;
    ASSERT_BINARYOP_OK (op, "op for GB_kron", GB0) ;
    ASSERT_MATRIX_OK (A, "A for GB_kron", GB0) ;
    ASSERT_MATRIX_OK (B, "B for GB_kron", GB0) ;

    // check domains and dimensions for C<M> = accum (C,T)
    GB_OK (GB_compatible (C->type, C, M, accum, op->ztype, Context)) ;

    // T=op(A,B) via op operator, so A and B must be compatible with z=op(a,b)
    GB_OK (GB_BinaryOp_compatible (op, NULL, A->type, B->type,
        GB_ignore_code, Context)) ;

    // delete any lingering zombies and assemble any pending tuples in A and B,
    // so that cnz = nnz(A) * nnz(B) can be computed.  Updates of C and M are
    // done after this check.
    GB_MATRIX_WAIT (A) ;
    GB_MATRIX_WAIT (B) ;

    // check the dimensions of C
    int64_t anrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t ancols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    int64_t bnrows = (B_transpose) ? GB_NCOLS (B) : GB_NROWS (B) ;
    int64_t bncols = (B_transpose) ? GB_NROWS (B) : GB_NCOLS (B) ;
    GrB_Index cnrows, cncols, cnz = 0 ;
    bool ok = GB_Index_multiply (&cnrows, anrows,  bnrows) ;
    ok = ok && GB_Index_multiply (&cncols, ancols,  bncols) ;
    ok = ok && GB_Index_multiply (&cnz, GB_NNZ (A), GB_NNZ (B)) ;
    if (!ok || GB_NROWS (C) != cnrows || GB_NCOLS (C) != cncols)
    { 
        GB_ERROR (GrB_DIMENSION_MISMATCH, "%s:\n"
            "output is " GBd "-by-" GBd "; must be " GBu "-by-" GBu "\n"
            "first input is " GBd "-by-" GBd "%s with " GBd " entries\n"
            "second input is " GBd "-by-" GBd "%s with " GBd " entries",
            ok ? "Dimensions not compatible:" : "Problem too large:",
            GB_NROWS (C), GB_NCOLS (C), cnrows, cncols,
            anrows, ancols, A_transpose ? " (transposed)" : "", GB_NNZ (A),
            bnrows, bncols, B_transpose ? " (transposed)" : "", GB_NNZ (B)) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    //--------------------------------------------------------------------------
    // transpose A and B if requested
    //--------------------------------------------------------------------------

    bool T_is_csc = C->is_csc ;
    if (T_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }
    if (T_is_csc != B->is_csc)
    { 
        // Flip the sense of B_transpose
        B_transpose = !B_transpose ;
    }

    if (!T_is_csc)
    { 
        if (GB_OP_IS_POSITIONAL (op))
        { 
            // positional ops must be flipped, with i and j swapped
            op = GB_positional_binop_ijflip (op) ;
        }
    }

    // TODO: if A, B are pattern: do not compute values of AT=A', BT=B'
    bool A_is_pattern, B_is_pattern ;
    GB_AxB_pattern (&A_is_pattern, &B_is_pattern, false, op->opcode) ;

    if (A_transpose)
    {
        // AT = A' and typecast to op->xtype
        // transpose: typecast, no op, not in-place
        GBURBLE ("(A transpose) ") ;
        GB_OK (GB_transpose (&AT, A_is_pattern ? A->type : op->xtype, T_is_csc,
            A, NULL, NULL, NULL, false, Context)) ;
        ASSERT_MATRIX_OK (A , "A after AT kron", GB0) ;
        ASSERT_MATRIX_OK (AT, "AT kron", GB0) ;
    }

    if (B_transpose)
    {
        // BT = B' and typecast to op->ytype
        // transpose: typecast, no op, not in-place
        GBURBLE ("(B transpose) ") ;
        GB_OK (GB_transpose (&BT, B_is_pattern ? B->type : op->ytype, T_is_csc,
            B, NULL, NULL, NULL, false, Context)) ;
        ASSERT_MATRIX_OK (BT, "BT kron", GB0) ;
    }

    //--------------------------------------------------------------------------
    // T = kron(A,B)
    //--------------------------------------------------------------------------

    GrB_Matrix T ;
    GB_OK (GB_kroner (&T, T_is_csc, op,
        A_transpose ? AT : A, A_is_pattern,
        B_transpose ? BT : B, B_is_pattern, Context)) ;

    // free workspace
    GB_FREE_ALL ;

    ASSERT_MATRIX_OK (T, "T = kron(A,B)", GB0) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Mask_struct, Context)) ;
}


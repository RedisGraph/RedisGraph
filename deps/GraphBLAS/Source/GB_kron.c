//------------------------------------------------------------------------------
// GB_kron: C<M> = accum (C, kron(A,B))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C, kron(A,B))

// The input matrices A and B are optionally transposed.

#include "GB_kron.h"
#include "GB_transpose.h"
#include "GB_accum_mask.h"

GrB_Info GB_kron                    // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '*' for kron(A,B)
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

    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (B) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    GB_RETURN_IF_FAULTY (accum) ;

    ASSERT_MATRIX_OK (C, "C input for GB_kron", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_kron", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_kron", GB0) ;
    ASSERT_BINARYOP_OK (op, "op for GB_kron", GB0) ;
    ASSERT_MATRIX_OK (A, "A for GB_kron", GB0) ;
    ASSERT_MATRIX_OK (B, "B for GB_kron", GB0) ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, M, accum, op->ztype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // T=op(A,B) via op operator, so A and B must be compatible with z=op(a,b)
    info = GB_BinaryOp_compatible (op, NULL, A->type, B->type,
        GB_ignore_code, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // delete any lingering zombies and assemble any pending tuples in A and B,
    // so that cnz = nnz(A) * nnz(B) can be computed.  Updates of C and M are
    // done after this check.
    GB_WAIT (A) ;
    GB_WAIT (B) ;

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
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG, "%s:\n"
            "output is "GBd"-by-"GBd"; must be "GBu"-by-"GBu"\n"
            "first input is "GBd"-by-"GBd"%s with "GBd" entries\n"
            "second input is "GBd"-by-"GBd"%s with "GBd" entries",
            ok ? "Dimensions not compatible:" : "Problem too large:",
            GB_NROWS (C), GB_NCOLS (C), cnrows, cncols,
            anrows, ancols, A_transpose ? " (transposed)" : "", GB_NNZ (A),
            bnrows, bncols, B_transpose ? " (transposed)" : "", GB_NNZ (B)))) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    // GB_WAIT (C) ;
    GB_WAIT (M) ;

    //--------------------------------------------------------------------------
    // transpose A and B if requested
    //--------------------------------------------------------------------------

    bool is_csc = C->is_csc ;
    if (is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }
    if (is_csc != B->is_csc)
    { 
        // Flip the sense of B_transpose
        B_transpose = !B_transpose ;
    }

    GrB_Matrix AT = NULL ;
    if (A_transpose)
    {
        // AT = A' and typecast to op->xtype
        // transpose: typecast, no op, not in place
        GBBURBLE ("(A transpose) ") ;
        info = GB_transpose (&AT, op->xtype, is_csc, A, NULL, Context) ;
        if (info != GrB_SUCCESS)
        { 
            return (info) ;
        }
        ASSERT_MATRIX_OK (A , "A after AT kron", GB0) ;
        ASSERT_MATRIX_OK (AT, "AT kron", GB0) ;
    }

    GrB_Matrix BT = NULL ;
    if (B_transpose)
    {
        // BT = B' and typecast to op->ytype
        // transpose: typecast, no op, not in place
        GBBURBLE ("(B transpose) ") ;
        info = GB_transpose (&BT, op->ytype, is_csc, B, NULL, Context) ;
        if (info != GrB_SUCCESS)
        { 
            GB_MATRIX_FREE (&AT) ;
            return (info) ;
        }
        ASSERT_MATRIX_OK (BT, "BT kron", GB0) ;
    }

    //--------------------------------------------------------------------------
    // T = kron(A,B)
    //--------------------------------------------------------------------------

    GrB_Matrix T ;
    info = GB_kroner (&T, C->is_csc, op,
        A_transpose ? AT : A, B_transpose ? BT : B, Context) ;

    // free workspace
    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&BT) ;

    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    ASSERT_MATRIX_OK (T, "T = kron(A,B)", GB0) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Mask_struct, Context)) ;
}


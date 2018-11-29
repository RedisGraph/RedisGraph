//------------------------------------------------------------------------------
// GB_eWise: C<M> = accum (C, A+B) or A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,A+B), A.*B and variations.

// The input matrices A and B are optionally transposed.

// Not user-callable.  Does the work for four user-callable functions:
//      GrB_eWiseAdd_*_BinaryOp
//      GrB_eWiseMult_*_BinaryOp

#include "GB.h"

GrB_Info GB_eWise                   // C<M> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, complement the mask M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '+' for C=A+B, or .* for A.*B
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    const bool eWiseAdd,            // if true, do set union (like A+B),
                                    // otherwise do intersection (like A.*B)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK3 (C, M, A, B)) ;

    GB_RETURN_IF_FAULTY (accum) ;

    ASSERT_OK (GB_check (C, "C input for GB_eWise", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for GB_eWise", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_eWise", GB0)) ;
    ASSERT_OK (GB_check (op, "op for GB_eWise", GB0)) ;
    ASSERT_OK (GB_check (A, "A for GB_eWise", GB0)) ;
    ASSERT_OK (GB_check (B, "B for GB_eWise", GB0)) ;

    // T has the same type as the output z for z=op(a,b)
    GrB_Type T_type = op->ztype ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, M, accum, T_type, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // T=op(A,B) via op operator, so A and B must be compatible with z=op(a,b)
    info = GB_BinaryOp_compatible (op, NULL, A->type, B->type, 0, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    if (eWiseAdd)
    {
        // C = A is done for entries in A but not C
        if (!GB_Type_compatible (C->type, A->type))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "first input of type [%s]\n"
                "cannot be typecast to final output of type [%s]",
                A->type->name, C->type->name))) ;
        }
        // C = B is done for entries in B but not C
        if (!GB_Type_compatible (C->type, B->type))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "second input of type [%s]\n"
                "cannot be typecast to final output of type [%s]",
                B->type->name, C->type->name))) ;
        }
    }

    // check the dimensions
    int64_t anrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t ancols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    int64_t bnrows = (B_transpose) ? GB_NCOLS (B) : GB_NROWS (B) ;
    int64_t bncols = (B_transpose) ? GB_NROWS (B) : GB_NCOLS (B) ;
    int64_t cnrows = GB_NROWS (C) ;
    int64_t cncols = GB_NCOLS (C) ;
    if (anrows != bnrows || ancols != bncols ||
        cnrows != anrows || cncols != bncols)
    { 
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "first input is "GBd"-by-"GBd"%s\n"
            "second input is "GBd"-by-"GBd"%s",
            cnrows, cncols,
            anrows, ancols, A_transpose ? " (transposed)" : "",
            bnrows, bncols, B_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty mask M is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;
    GB_WAIT (B) ;

    //--------------------------------------------------------------------------
    // handle CSR and CSC formats
    //--------------------------------------------------------------------------

    // CSC/CSR format of T is same as C.  Conform A and B to the format of C.

    bool is_csc = C->is_csc ;
    if (is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose.  For example, if C is CSC and A is
        // CSR, and A_transpose is true, then C=A'+B is being computed.  But
        // this is the same as C=A+B where A is treated as if it is CSC.
        // GB_add and GB_emult are agnostic to the CSR/CSC
        // format of their inputs.
        A_transpose = !A_transpose ;
    }

    if (is_csc != B->is_csc)
    { 
        // Flip the sense of B_transpose.
        B_transpose = !B_transpose ;
    }

    //--------------------------------------------------------------------------
    // T = A+B, A'+B, A+B', or A'+B'
    //--------------------------------------------------------------------------

    GrB_Matrix T ;

    if (A_transpose)
    {
        if (B_transpose)
        {

            //------------------------------------------------------------------
            // T = A'+B'
            //------------------------------------------------------------------

            // T = A + B, with flipped CSR/CSC format so GB_accum_mask does C=T'
            if (eWiseAdd)
            { 
                info = GB_add (&T, T_type, !is_csc, A, B, op, Context) ;
            }
            else
            { 
                info = GB_emult (&T, T_type, !is_csc, A, B, op, Context) ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // T = A'+B
            //------------------------------------------------------------------

            // FUTURE: for emult, if B is much sparser than A then do T'=A+B'

            // AT = A'
            GrB_Matrix AT = NULL ;
            // transpose: no typecast, no op, not in place
            info = GB_transpose (&AT, NULL, is_csc, A, NULL, Context) ;
            if (info != GrB_SUCCESS)
            { 
                return (info) ;
            }

            // T = AT + B
            if (eWiseAdd)
            { 
                info = GB_add (&T, T_type, is_csc, AT, B, op, Context) ;
            }
            else
            { 
                info = GB_emult (&T, T_type, is_csc, AT, B, op, Context) ;
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

            // FUTURE: for emult, if A is much sparser than B then do T'=A'+B

            // BT = B'
            GrB_Matrix BT = NULL ;
            // transpose: no typecast, no op, not in place
            info = GB_transpose (&BT, NULL, is_csc, B, NULL, Context) ;
            if (info != GrB_SUCCESS)
            { 
                return (info) ;
            }

            // T = A + BT
            if (eWiseAdd)
            { 
                info = GB_add (&T, T_type, is_csc, A, BT, op, Context) ;
            }
            else
            { 
                info = GB_emult (&T, T_type, is_csc, A, BT, op, Context) ;
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
                info = GB_add (&T, T_type, is_csc, A, B, op, Context) ;
            }
            else
            { 
                info = GB_emult (&T, T_type, is_csc, A, B, op, Context) ;
            }
        }
    }

    if (info != GrB_SUCCESS)
    { 
        ASSERT (T == NULL) ;            // T has already been freed
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the M
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Context)) ;
}


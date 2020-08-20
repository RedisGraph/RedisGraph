//------------------------------------------------------------------------------
// GB_ewise: C<M> = accum (C, A+B) or A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,A+B), A.*B and variations.  The input matrices A and B are
// optionally transposed.  Does the work for GrB_eWiseAdd_* and
// GrB_eWiseMult_*.  Handles all cases of the mask.

#include "GB_ewise.h"
#include "GB_add.h"
#include "GB_emult.h"
#include "GB_transpose.h"
#include "GB_accum_mask.h"
#include "GB_dense.h"

#define GB_FREE_ALL         \
{                           \
    GB_MATRIX_FREE (&T) ;   \
    GB_MATRIX_FREE (&AT) ;  \
    GB_MATRIX_FREE (&BT) ;  \
    GB_MATRIX_FREE (&MT) ;  \
}

GrB_Info GB_ewise                   // C<M> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, complement the mask M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '+' for C=A+B, or .* for A.*B
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    bool eWiseAdd,                  // if true, do set union (like A+B),
                                    // otherwise do intersection (like A.*B)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M, A, and/or B

    GrB_Info info ;
    GrB_Matrix MT = NULL, BT = NULL, AT = NULL, T = NULL ;

    GB_RETURN_IF_FAULTY (accum) ;

    ASSERT_MATRIX_OK (C, "C input for GB_ewise", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_ewise", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_ewise", GB0) ;
    ASSERT_BINARYOP_OK (op, "op for GB_ewise", GB0) ;
    ASSERT_MATRIX_OK (A, "A for GB_ewise", GB0) ;
    ASSERT_MATRIX_OK (B, "B for GB_ewise", GB0) ;

    // T has the same type as the output z for z=op(a,b)
    GrB_Type T_type = op->ztype ;

    // check domains and dimensions for C<M> = accum (C,T)
    GB_OK (GB_compatible (C->type, C, M, accum, T_type, Context)) ;

    // T=op(A,B) via op operator, so A and B must be compatible with z=op(a,b)
    GB_OK (GB_BinaryOp_compatible (op, NULL, A->type, B->type,
        GB_ignore_code, Context)) ;

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
    // GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;
    GB_WAIT (B) ;

    //--------------------------------------------------------------------------
    // handle CSR and CSC formats
    //--------------------------------------------------------------------------

    // CSC/CSR format of T is same as C.  Conform A and B to the format of C.

    bool C_is_csc = C->is_csc ;
    if (C_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose.  For example, if C is CSC and A is
        // CSR, and A_transpose is true, then C=A'+B is being computed.  But
        // this is the same as C=A+B where A is treated as if it is CSC.
        A_transpose = !A_transpose ;
    }

    if (C_is_csc != B->is_csc)
    { 
        // Flip the sense of B_transpose.
        B_transpose = !B_transpose ;
    }

    if (A_transpose && B_transpose)
    { 
        // T=A'+B' replaced with T=(A+B)'
        A_transpose = false ;
        B_transpose = false ;
        C_is_csc = !C_is_csc ;
    }

    //--------------------------------------------------------------------------
    // determine if any matrices are dense
    //--------------------------------------------------------------------------

    bool C_is_dense = GB_is_dense (C) && !GB_PENDING_OR_ZOMBIES (C) ;
    bool A_is_dense = GB_is_dense (A) ;
    bool B_is_dense = GB_is_dense (B) ;
    bool M_is_dense = GB_is_dense (M) ;

    if (C_is_dense) { GBBURBLE ("(C dense) ") ; }
    if (A_is_dense) { GBBURBLE ("(A dense) ") ; }
    if (B_is_dense) { GBBURBLE ("(B dense) ") ; }
    if (M_is_dense) { GBBURBLE ("(M dense) ") ; }

    //--------------------------------------------------------------------------
    // decide when to apply the mask
    //--------------------------------------------------------------------------

    // GB_add and GB_emult can apply any non-complemented mask, but it is
    // faster to exploit the mask in GB_add / GB_emult only when it is very
    // sparse compared with A and B, or (in special cases) when it is easy
    // to apply.

    // check the CSR/CSC format of M
    bool M_is_csc = (M == NULL) ? C_is_csc : M->is_csc ;
    bool mask_applied = false ;
    GrB_Matrix M1 = NULL ;

    if (M != NULL && !Mask_comp)
    {
        // mask is present, not complemented; see if it is quick or easy to use.
        // it may be a structural or valued mask.
        bool mask_is_easy = (A_is_dense || (A == M))    // A is easy
                         && (B_is_dense || (B == M)) ;  // and B is easy
        bool mask_is_very_sparse = GB_MASK_VERY_SPARSE (M, A, B) ;
        if (mask_is_easy || mask_is_very_sparse)
        {
            // the mask is present, not complemented, and very sparse or easy
            // to exploit ; use it during GB_add and GB_emult to reduce memory
            // and work.
            M1 = M ;
            if (C_is_csc != M_is_csc)
            { 
                GBBURBLE ("(M transpose) ") ;
                GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M, NULL, Context));
                M1 = MT ;
            }
            mask_applied = true ;
            if (mask_is_easy)
            { 
                GBBURBLE ("(mask is easy) ") ;
            }
            else // mask_is_very_sparse
            { 
                GBBURBLE ("(mask applied) ") ;
            }
        }
        else
        { 
            GBBURBLE ("(mask later) ") ;
        }
    }

    //--------------------------------------------------------------------------
    // transpose A if needed
    //--------------------------------------------------------------------------

    GrB_Matrix A1 = A ;
    if (A_transpose)
    { 
        // AT = A'
        // transpose: no typecast, no op, not in place
        GBBURBLE ("(A transpose) ") ;
        GB_OK (GB_transpose (&AT, NULL, C_is_csc, A, NULL, Context)) ;
        A1 = AT ;
    }

    //--------------------------------------------------------------------------
    // transpose B if needed
    //--------------------------------------------------------------------------

    GrB_Matrix B1 = B ;
    if (B_transpose)
    { 
        // BT = B'
        // transpose: no typecast, no op, not in place
        GBBURBLE ("(B transpose) ") ;
        GB_OK (GB_transpose (&BT, NULL, C_is_csc, B, NULL, Context)) ;
        B1 = BT ;
    }

    //--------------------------------------------------------------------------
    // special cases
    //--------------------------------------------------------------------------

    // FUTURE::: handle more special cases

    if (A_is_dense && B_is_dense)
    { 
        // no need to use eWiseAdd if both A and B are dense
        eWiseAdd = false ;
    }

    bool no_typecast =
        (op->ztype == C->type)              // no typecasting of C
        && (op->xtype == A1->type)          // no typecasting of A
        && (op->ytype == B1->type) ;        // no typecasting of B

    #ifndef GBCOMPACT

        // FUTURE: for sssp12:
        // C<A> = A+B where C is sparse and B is dense;
        // mask is structural, not complemented, C_replace is false.
        // C is not empty.  Use a kernel that computes T<A>=A+B
        // where T starts out empty; just iterate over the entries in A.

    if (A_is_dense                          // A and B are dense
        && B_is_dense
        && (M == NULL) && !Mask_comp        // no mask
        && (C->is_csc == C_is_csc)          // no transpose of C
        && no_typecast                      // no typecasting
        && (op->opcode < GB_USER_opcode)    // not a user-defined operator
        )
    {

        if (C_is_dense                      // C is dense
        && accum == op                      // accum is same as the op
        && (op->opcode >= GB_MIN_opcode)    // subset of binary operators
        && (op->opcode <= GB_RDIV_opcode))
        { 

            //------------------------------------------------------------------
            // C += A+B where all 3 matrices are dense
            //------------------------------------------------------------------

            // C_replace is ignored
            GBBURBLE ("dense C+=A+B ") ;
            GB_dense_ewise3_accum (C, A1, B1, op, Context) ;    // cannot fail
            GB_FREE_ALL ;
            ASSERT_MATRIX_OK (C, "C output for GB_ewise, dense C+=A+B", GB0) ;
            return (GrB_SUCCESS) ;

        }
        else if (accum == NULL)             // no accum
        { 

            //------------------------------------------------------------------
            // C = A+B where A and B are dense (C is anything)
            //------------------------------------------------------------------

            // C_replace is ignored
            GBBURBLE ("dense C=A+B ") ;
            info = GB_dense_ewise3_noaccum (C, C_is_dense, A1, B1, op, Context);
            GB_FREE_ALL ;
            if (info == GrB_SUCCESS)
            {
                ASSERT_MATRIX_OK (C, "C output for GB_ewise, dense C=A+B", GB0);
            }
            return (info) ;
        }
    }

    #endif

    //--------------------------------------------------------------------------
    // T = A+B or A.*B
    //--------------------------------------------------------------------------

    if (eWiseAdd)
    { 
        GB_OK (GB_add (&T, T_type, C_is_csc, M1, Mask_struct, A1, B1, op,
            Context)) ;
    }
    else
    { 
        GB_OK (GB_emult (&T, T_type, C_is_csc, M1, Mask_struct, A1, B1, op,
            Context)) ;
    }

    //--------------------------------------------------------------------------
    // free the transposed matrices
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&BT) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask
    //--------------------------------------------------------------------------

    if ((accum == NULL) && (C->is_csc == T->is_csc)
        && (M == NULL || (M != NULL && mask_applied))
        && (C_replace || GB_NNZ_UPPER_BOUND (C) == 0))
    { 
        // C = 0 ; C = (ctype) T ; with the same CSR/CSC format.  The mask M
        // (if any) has already been applied.  If C is also empty, or to be
        // cleared anyway, and if accum is not present, then T can be
        // transplanted directly into C, as C = (ctype) T, typecasting if
        // needed.  If no typecasting is done then this takes no time at all
        // and is a pure transplant.  Also conform C to its desired
        // hypersparsity.
        GB_MATRIX_FREE (&MT) ;
        return (GB_transplant_conform (C, C->type, &T, Context)) ;
    }
    else
    { 
        // C<M> = accum (C,T)
        // GB_accum_mask also conforms C to its desired hypersparsity
        info = GB_accum_mask (C, M, MT, accum, &T, C_replace, Mask_comp,
            Mask_struct, Context) ;
        GB_MATRIX_FREE (&MT) ;
        return (info) ;
    }
}


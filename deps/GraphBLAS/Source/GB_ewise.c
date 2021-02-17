//------------------------------------------------------------------------------
// GB_ewise: C<M> = accum (C, A+B) or A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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
    GB_Matrix_free (&T) ;   \
    GB_Matrix_free (&AT) ;  \
    GB_Matrix_free (&BT) ;  \
    GB_Matrix_free (&MT) ;  \
}

GrB_Info GB_ewise                   // C<M> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, complement the mask M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op_in,       // defines '+' for C=A+B, or .* for A.*B
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

    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;

    ASSERT_MATRIX_OK (C, "C input for GB_ewise", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_ewise", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_ewise", GB0) ;
    ASSERT_BINARYOP_OK (op_in, "op for GB_ewise", GB0) ;
    ASSERT_MATRIX_OK (A, "A for GB_ewise", GB0) ;
    ASSERT_MATRIX_OK (B, "B for GB_ewise", GB0) ;

    // T has the same type as the output z for z=op(a,b)
    GrB_BinaryOp op = op_in ;
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
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "First input of type [%s]\n"
                "cannot be typecast to final output of type [%s]",
                A->type->name, C->type->name) ;
        }
        // C = B is done for entries in B but not C
        if (!GB_Type_compatible (C->type, B->type))
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Second input of type [%s]\n"
                "cannot be typecast to final output of type [%s]",
                B->type->name, C->type->name) ;
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
        GB_ERROR (GrB_DIMENSION_MISMATCH,
            "Dimensions not compatible:\n"
            "output is " GBd "-by-" GBd "\n"
            "first input is " GBd "-by-" GBd "%s\n"
            "second input is " GBd "-by-" GBd "%s",
            cnrows, cncols,
            anrows, ancols, A_transpose ? " (transposed)" : "",
            bnrows, bncols, B_transpose ? " (transposed)" : "") ;
    }

    // quick return if an empty mask M is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    //--------------------------------------------------------------------------
    // handle CSR and CSC formats
    //--------------------------------------------------------------------------

    GB_Opcode opcode = op->opcode ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;

    // CSC/CSR format of T is same as C.  Conform A and B to the format of C.
    bool T_is_csc = C->is_csc ;
    if (T_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose.  For example, if C is CSC and A is
        // CSR, and A_transpose is true, then C=A'+B is being computed.  But
        // this is the same as C=A+B where A is treated as if it is CSC.
        A_transpose = !A_transpose ;
    }

    if (T_is_csc != B->is_csc)
    { 
        // Flip the sense of B_transpose.
        B_transpose = !B_transpose ;
    }

    if (A_transpose && B_transpose)
    { 
        // T=A'+B' is not computed.  Instead, T=A+B is computed first,
        // and then C = T' is computed.
        A_transpose = false ;
        B_transpose = false ;
        // The CSC format of T and C now differ.
        T_is_csc = !T_is_csc ;
    }

    if (!T_is_csc)
    { 
        if (op_is_positional)
        { 
            // positional ops must be flipped, with i and j swapped
            op = GB_positional_binop_ijflip (op) ;
            opcode = op->opcode ;
        }
    }

    //--------------------------------------------------------------------------
    // determine if any matrices are dense or full
    //--------------------------------------------------------------------------

    bool C_is_dense = GB_is_dense (C) && !GB_PENDING_OR_ZOMBIES (C) ;
    bool A_is_dense = GB_is_dense (A) ;
    bool B_is_dense = GB_is_dense (B) ;

    //--------------------------------------------------------------------------
    // decide when to apply the mask
    //--------------------------------------------------------------------------

    // GB_add and GB_emult can apply any non-complemented mask, but it is
    // faster to exploit the mask in GB_add / GB_emult only when it is very
    // sparse compared with A and B, or (in special cases) when it is easy
    // to apply.

    // check the CSR/CSC format of M
    bool M_is_csc = (M == NULL) ? T_is_csc : M->is_csc ;

    //--------------------------------------------------------------------------
    // transpose M if needed
    //--------------------------------------------------------------------------

    GrB_Matrix M1 = M ;
    bool M_transpose = (T_is_csc != M_is_csc) ;
    if (M_transpose)
    { 
        // MT = M'
        // transpose: no typecast, no op, not in-place
        GBURBLE ("(M transpose) ") ;
        GB_OK (GB_transpose (&MT, GrB_BOOL, T_is_csc, M,
            NULL, NULL, NULL, false, Context)) ;
        M1 = MT ;
    }

    //--------------------------------------------------------------------------
    // transpose A if needed
    //--------------------------------------------------------------------------

    GrB_Matrix A1 = A ;
    if (A_transpose)
    { 
        // AT = A'
        // transpose: no typecast, no op, not in-place
        GBURBLE ("(A transpose) ") ;
        GB_OK (GB_transpose (&AT, NULL, T_is_csc, A,
            NULL, NULL, NULL, false, Context)) ;
        A1 = AT ;
        ASSERT_MATRIX_OK (AT, "AT from transpose", GB0) ;
    }

    //--------------------------------------------------------------------------
    // transpose B if needed
    //--------------------------------------------------------------------------

    GrB_Matrix B1 = B ;
    if (B_transpose)
    { 
        // BT = B'
        // transpose: no typecast, no op, not in-place
        GBURBLE ("(B transpose) ") ;
        GB_OK (GB_transpose (&BT, NULL, T_is_csc, B,
            NULL, NULL, NULL, false, Context)) ;
        B1 = BT ;
    }

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    // TODO: delay the unjumbling of A1 and B1.  If either is bitmap/full,
    // then they other can remain jumbled.
    GB_MATRIX_WAIT (M1) ;       // cannot be jumbled
    GB_MATRIX_WAIT (A1) ;       // cannot be jumbled
    GB_MATRIX_WAIT (B1) ;       // cannot be jumbled

    //--------------------------------------------------------------------------
    // special cases
    //--------------------------------------------------------------------------

    // FUTURE::: handle more special cases:
    // C<M>=A+B when C and A are dense, B is sparse.  M can be sparse.
    // C<M>=A+B when C and B are dense, A is sparse.  M can be sparse.
    // C<M>=A+B when C, A, and B are dense.  M can be sparse.
    // Also do:
    // C<M>+=A+B when C and A are dense, B is sparse.  M can be sparse.
    // C<M>+=A+B when C and B are dense, A is sparse.  M can be sparse.
    // C<M>+=A+B when C, A, and B are dense.  M can be sparse.
    // In all cases above, C remains dense and can be updated in-place
    // C_replace must be false.  M can be valued or structural.

    bool no_typecast =
        (op->ztype == C->type)              // no typecasting of C
        && (op->xtype == A1->type)          // no typecasting of A
        && (op->ytype == B1->type) ;        // no typecasting of B

    bool C_is_bitmap = GB_IS_BITMAP (C) ;
    bool M_is_bitmap = GB_IS_BITMAP (M) ;
    bool A_is_bitmap = GB_IS_BITMAP (A) ;
    bool B_is_bitmap = GB_IS_BITMAP (B) ;
    bool any_bitmap = C_is_bitmap || M_is_bitmap || A_is_bitmap || B_is_bitmap ;

    #ifndef GBCOMPACT

        // FUTURE: for sssp12:
        // C<A> = A+B where C is sparse and B is dense;
        // mask is structural, not complemented, C_replace is false.
        // C is not empty.  Use a kernel that computes T<A>=A+B
        // where T starts out empty; just iterate over the entries in A.

    if (A_is_dense                          // A and B are dense
        && B_is_dense
        && (M == NULL) && !Mask_comp        // no mask
        && (C->is_csc == T_is_csc)          // no transpose of C
        && no_typecast                      // no typecasting
        && (opcode < GB_USER_opcode)        // not a user-defined operator
        && (!op_is_positional)              // op is not positional
        && !any_bitmap)
    {

        if (C_is_dense                      // C is dense
        && accum == op                      // accum is same as the op
        && (opcode >= GB_MIN_opcode)        // subset of binary operators
        && (opcode <= GB_RDIV_opcode))
        { 

            //------------------------------------------------------------------
            // C += A+B where all 3 matrices are dense
            //------------------------------------------------------------------

            // C_replace is ignored
            GBURBLE ("dense C+=A+B ") ;
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
            GBURBLE ("dense C=A+B ") ;
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
    // T = A+B or A.*B, or with any mask M
    //--------------------------------------------------------------------------

    bool mask_applied = false ;

    if (eWiseAdd)
    { 

        //----------------------------------------------------------------------
        // T<any mask> = A+B
        //----------------------------------------------------------------------

        // TODO: check the mask condition in GB_add_sparsity.
        // Only exploit the mask in GB_add if it's more efficient than
        // exploiting it later, probably this condition:

            // (accum == NULL) && (C->is_csc == T->is_csc)
            // && (C_replace || GB_NNZ_UPPER_BOUND (C) == 0))

        // If that is true and the mask is applied, then T is transplanted as
        // the final C and the mask is no longer needed.  In this case, it
        // could be faster to exploit the mask duing GB_add.

        GB_OK (GB_add (&T, T_type, T_is_csc, M1, Mask_struct, Mask_comp,
            &mask_applied, A1, B1, op, Context)) ;
    }
    else
    { 

        //----------------------------------------------------------------------
        // T<any mask> = A.*B
        //----------------------------------------------------------------------

        // T can be returned with shallow components derived from its inputs A1
        // and/or B1.  In particular, if T is hypersparse, T->h may be a
        // shallow copy of A1->h, B1->h, or M1->h.  T is hypersparse if any
        // matrix A1, B1, or M1 are hypersparse.  Internally, T->h always
        // starts as a shallow copy of A1->h, B1->h, or M1->h, but it may be
        // pruned by GB_hypermatrix_prune, and thus no longer shallow.

#if 0
        // TODO: check whether or not to exploit the mask in GB_emult_sparsity.

        if (M != NULL && !Mask_comp)
        {
            // mask is present, not complemented; see if it is quick or easy to
            // use.  it may be a structural or valued mask.
            bool mask_is_easy = (A_is_dense || (A == M))    // A is easy
                             && (B_is_dense || (B == M)) ;  // and B is easy
            bool mask_is_very_sparse = GB_MASK_VERY_SPARSE (M, A, B) ;
            if (mask_is_easy || mask_is_very_sparse)
            {
                // the mask is present, not complemented, and very sparse or
                // easy to exploit ; use it during GB_add and GB_emult to
                // reduce memory and work.
                mask_applied = true ;
            }
            else
            {
                // do not apply the mask now
                M1 = NULL ;
                mask_applied = false ;
            }
        }
        else
        {
            // do not apply the mask now
            M1 = NULL ;
            mask_applied = false ;
        }
#endif

        GB_OK (GB_emult (&T, T_type, T_is_csc, M1, Mask_struct, Mask_comp,
            &mask_applied, A1, B1, op, Context)) ;

        //----------------------------------------------------------------------
        // transplant shallow content from AT, BT, or MT
        //----------------------------------------------------------------------

        // If T is hypersparse, T->h is always a shallow copy of A1->h, B1->h,
        // or M1->h.  Any of the three matrices A1, B1, or M1 may be temporary
        // transposes, AT, BT, and MT respectively.  If T->h is a shallow cpoy
        // of a temporary matrix, then flip the ownership of the T->h array,
        // from the temporary matrix into T, so that T->h is not freed when AT,
        // BT, and MT are freed.

        // GB_tranpose can return all kinds of shallow components, particularly
        // when transposing vectors.  It can return AT->h as shallow copy of
        // A->i, for example.

        if (T->h_shallow)
        {
            // T->h is shallow and T is hypersparse
            ASSERT (GB_IS_HYPERSPARSE (T)) ;
            // one of A1, B1, or M1 is hypersparse
            ASSERT (GB_IS_HYPERSPARSE (A1) || GB_IS_HYPERSPARSE (B1) ||
                    GB_IS_HYPERSPARSE (M1))
            if (A_transpose && T->h == A1->h)
            { 
                // A1 is the temporary matrix AT.  AT->h might itself be a
                // shallow copy of A->h or A->i, from GB_transpose.
                ASSERT (A1 == AT) ;
                T->h_shallow = AT->h_shallow ;
                AT->h_shallow = true ;
            }
            else if (B_transpose && T->h == B1->h)
            { 
                // B1 is the temporary matrix BT.  BT->h might itself be a
                // shallow copy of B->h or B->i, from GB_transpose.
                ASSERT (B1 == BT) ;
                T->h_shallow = BT->h_shallow ;
                BT->h_shallow = true ;
            }
            else if (M_transpose && T->h == M1->h)
            { 
                // M1 is the temporary matrix MT.  MT->h might itself be a
                // shallow copy of M->h or M->i, from GB_transpose.
                ASSERT (M1 == MT) ;
                T->h_shallow = MT->h_shallow ;
                MT->h_shallow = true ;
            }

            // T->h may still be shallow, but if so, it is a shallow copy of
            // some component of the user input matrices A, B, or M, and must
            // remain shallow.  A deep copy of it will be made when T->h is
            // transplanted into the result C.
            ASSERT (GB_IMPLIES (T->h_shallow,
                (T->h == A1->h || T->h == B1->h ||
                 (M1 != NULL && T->h == M1->h)))) ;
        }
    }

    //--------------------------------------------------------------------------
    // free the transposed matrices
    //--------------------------------------------------------------------------

    GB_Matrix_free (&AT) ;
    GB_Matrix_free (&BT) ;

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
        GB_Matrix_free (&MT) ;
        GB_OK (GB_transplant_conform (C, C->type, &T, Context)) ;
        return (GB_block (C, Context)) ;
    }
    else
    { 
        // C<M> = accum (C,T)
        // GB_accum_mask also conforms C to its desired hypersparsity
        info = GB_accum_mask (C, M, MT, accum, &T, C_replace, Mask_comp,
            Mask_struct, Context) ;
        GB_Matrix_free (&MT) ;
        return (info) ;
    }
}


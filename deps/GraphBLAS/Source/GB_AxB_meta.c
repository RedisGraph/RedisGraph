//------------------------------------------------------------------------------
// GB_AxB_meta: C<M>=A*B meta algorithm
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C, C<M>, C<!M> = A*B, A'*B, A*B', or A'*B' : both symbolic and numeric, with
// the optional mask matrix.  This function is called by GB_mxm only.  If the
// mask matrix is present, it can be regular or complemented, and either valued
// or structural.

// This algorithm may decide that it is more efficient to apply the mask later,
// in GB_accum_mask, after this matrix C is computed, in GB_mxm.  The result is
// either the T matrix in GB_mxm, or (if done in-place), the final output
// matrix C passed in from the user (C_in_place).

// The method is chosen automatically:  a gather/scatter saxpy method
// (Gustavson), a heap-based saxpy method, or a dot product method.  The
// AxB_method can modify this automatic choice, if set to a non-default value.
// AxB_method_used is DOT, SAXPY, or DEFAULT (the latter denotes the row/col
// scaling methods).

// FUTURE:: an outer-product method for C=A*B'

#define GB_FREE_ALL             \
{                               \
    GB_MATRIX_FREE (Chandle) ;  \
    GB_MATRIX_FREE (&AT) ;      \
    GB_MATRIX_FREE (&BT) ;      \
    GB_MATRIX_FREE (&MT) ;      \
}

#include "GB_mxm.h"
#include "GB_transpose.h"

GrB_Info GB_AxB_meta                // C<M>=A*B meta algorithm
(
    GrB_Matrix *Chandle,            // output matrix (if not done in place)
    GrB_Matrix C_in_place,          // input/output matrix, if done in place
    bool C_replace,                 // C matrix descriptor
    const bool C_is_csc,            // desired CSR/CSC format of C
    GrB_Matrix *MT_handle,          // return MT = M' to caller, if computed
    const GrB_Matrix M_in,          // mask for C<M> (not complemented)
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // accum operator for C_in_place += A*B
    const GrB_Matrix A_in,          // input matrix
    const GrB_Matrix B_in,          // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    bool A_transpose,               // if true, use A', else A
    bool B_transpose,               // if true, use B', else B
    bool flipxy,                    // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    bool *done_in_place,            // if true, C was computed in place
    GrB_Desc_Value AxB_method,      // for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK_OR_NULL (C_in_place, "C_in_place for meta A*B", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M_in, "M for meta A*B", GB0) ;
    ASSERT_MATRIX_OK (A_in, "A_in for meta A*B", GB0) ;
    ASSERT_MATRIX_OK (B_in, "B_in for meta A*B", GB0) ;
    ASSERT (!GB_PENDING (M_in)) ; ASSERT (!GB_ZOMBIES (M_in)) ;
    ASSERT (!GB_PENDING (A_in)) ; ASSERT (!GB_ZOMBIES (A_in)) ;
    ASSERT (!GB_PENDING (B_in)) ; ASSERT (!GB_ZOMBIES (B_in)) ;
    ASSERT_SEMIRING_OK (semiring, "semiring for numeric A*B", GB0) ;
    ASSERT (mask_applied != NULL) ;
    ASSERT (AxB_method_used != NULL) ;
    ASSERT (Chandle != NULL) ;

    (*Chandle) = NULL ;
    if (MT_handle != NULL)
    { 
        (*MT_handle) = NULL ;
    }

    GrB_Info info ;

    GrB_Matrix AT = NULL ;
    GrB_Matrix BT = NULL ;
    GrB_Matrix MT = NULL ;

    (*mask_applied) = false ;
    (*done_in_place) = false ;
    (*AxB_method_used) = GxB_DEFAULT ;

    if (AxB_method == GxB_AxB_HEAP)
    { 
        // FUTURE::: Heap method not yet reinstalled; using Hash instead
        AxB_method = GxB_AxB_HASH ;
    }

    //--------------------------------------------------------------------------
    // see if the work can be done in place
    //--------------------------------------------------------------------------

    // C can be computed in place if it is already dense, and if it is
    // guaranteed to remain dense after the computation is done.  This case
    // requires the accum operator to be present and it must match the monoid
    // of the semiring.  C_replace must be false, or effectively false.
    // Finally, C must not transposed on output.

    bool can_do_in_place = false ;
    if (C_in_place != NULL && accum != NULL)
    { 
        // check if C_in_place is competely dense:  all entries present and no
        // pending work
        bool C_is_dense = !GB_PENDING_OR_ZOMBIES (C_in_place)
            && GB_is_dense (C_in_place) ;

        // accum must be present, and must match the monoid of the semiring,
        // and the ztype of the monoid must match the type of C
        bool accum_is_monoid = (accum == semiring->add->op) 
            && (C_in_place->type == accum->ztype) ;

        // C += A*B with C_replace ignored (effectively false)
        // C<M> += A*B with C_replace false
        // C<!M> += A*B with C_replace false
        can_do_in_place =
            C_is_dense
            && accum_is_monoid
            && ((M_in == NULL) || (M_in != NULL && !C_replace)) ;

        // C must also not be transposed on output; see below.
        // Nor can it be aliased with any input matrix.
    }

    //--------------------------------------------------------------------------
    // handle the CSR/CSC formats of C, M, A, and B
    //--------------------------------------------------------------------------

    // On input, A and/or B can be transposed, and all four matrices can be in
    // either CSR or CSC format, in any combination.  This gives a total of 64
    // possible combinations.  However, a CSR matrix that is transposed is just
    // the same as a non-transposed CSC matrix.

    // Use transpose to handle the CSR/CSC format.  If C is desired in CSR
    // format, treat it as if it were in format CSC but transposed.
    bool C_transpose = !C_is_csc ;

    // If the mask is not present, then treat it as having the same CSR/CSC
    // format as C.
    bool M_is_csc = (M_in == NULL) ? C_is_csc : M_in->is_csc ;

    // Treat M just like C.  If M is in CSR format, treat it as if it were CSC
    // but transposed, since there are no descriptors that transpose C or M.
    bool M_transpose = !M_is_csc ;

    // A can be transposed, and can also be in CSR or CSC format.  If A is in
    // CSR, treat it as A' in CSC, and if A' is in CSR, treat it as A in CSC.
    if (!A_in->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    // B is treated just like A
    if (!B_in->is_csc)
    { 
        // Flip the sense of A_transpose
        B_transpose = !B_transpose ;
    }

        // Now all matrices C, M_in, A_in, and B_in, can be treated as if they
        // were all in CSC format, except any of them can be transposed.  There
        // are now 16 cases to handle, where M, A, and B are M_in, A_in, and
        // B_in and all matrices are CSR/CSC agnostic, and where C has not yet
        // been created.

        //      C <M > = A  * B
        //      C <M'> = A  * B
        //      C'<M > = A  * B
        //      C'<M'> = A  * B

        //      C <M > = A  * B'
        //      C <M'> = A  * B'
        //      C'<M > = A  * B'
        //      C'<M'> = A  * B'

        //      C <M > = A' * B
        //      C <M'> = A' * B
        //      C'<M > = A' * B
        //      C'<M'> = A' * B

        //      C <M > = A' * B'
        //      C <M'> = A' * B'
        //      C'<M > = A' * B'
        //      C'<M'> = A' * B'

    //--------------------------------------------------------------------------
    // swap_rule: decide if C or C' should be computed
    //--------------------------------------------------------------------------

    // This function can compute C or C', by setting C->is_csc as the negation
    // of the desired format C_is_csc.  This ensures that GB_accum_mask will
    // transpose C when this function is done.

    // For these 4 cases, the swap_rule is true:

        // C' = A'*B'       becomes C = B*A
        // C' = A'*B        becomes C = B'*A
        // C' = A*B'        becomes C = B*A'
        // C  = A'*B'       becomes C = (B*A)'

    // For these 4 cases, the swap_rule is false:

        // C' = A*B         C = (A*B)'
        // C  = A'*B        use as-is
        // C  = A*B'        use as-is
        // C  = A*B         use as-is

    // This rule is the same as that used by SuiteSparse/MATLAB_Tools/SSMULT,
    // which is the built-in sparse matrix multiply function in MATLAB.

    bool swap_rule =
        ( C_transpose &&  A_transpose &&  B_transpose) ||   // C' = A'*B'
        ( C_transpose &&  A_transpose && !B_transpose) ||   // C' = A'*B
        ( C_transpose && !A_transpose &&  B_transpose) ||   // C' = A*B'
        (!C_transpose &&  A_transpose &&  B_transpose) ;    // C  = A'*B'

    GrB_Matrix A, B ;
    bool atrans, btrans ;

    if (swap_rule)
    { 
        // Replace C'=(A'*B') with C=B*A, and so on.  Swap A and B and transose
        // them, transpose M, negate flipxy, and transpose M and C.
        A = B_in ; atrans = !B_transpose ;
        B = A_in ; btrans = !A_transpose ;
        flipxy = !flipxy ;
        M_transpose = !M_transpose ;
        C_transpose = !C_transpose ;
    }
    else
    { 
        // use the input matrices as-is
        A = A_in ; atrans = A_transpose ;
        B = B_in ; btrans = B_transpose ;
    }

    ASSERT_MATRIX_OK (A, "final A for A*B", GB0) ;
    ASSERT_MATRIX_OK (B, "final B for A*B", GB0) ;

    //--------------------------------------------------------------------------
    // explicitly transpose the mask
    //--------------------------------------------------------------------------

    // all uses of GB_transpose below:
    // transpose: typecast, no op, not in place

    GrB_Matrix M ;
    bool M_transposed ;

    if (M_transpose && M_in != NULL)
    { 
        // MT = M_in' also typecasting to boolean.  It is not freed here
        // unless an error occurs, but is returned to the caller.
        GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M_in, NULL, Context)) ;
        M = MT ;
        M_transposed = true ;
    }
    else
    { 
        // M_in can be used as-is; it may be NULL
        M = M_in ;
        M_transposed = false ;
    }

    ASSERT_MATRIX_OK_OR_NULL (M, "final M for A*B", GB0) ;

    //--------------------------------------------------------------------------
    // check additional conditions for in-place computation of C
    //--------------------------------------------------------------------------

    if (can_do_in_place)
    {
        // C cannot be done in place if it is aliased with any input matrix.
        // Also cannot compute C in place (yet) if it is to be transposed.
        bool C_aliased =
            GB_aliased (C_in_place, M) ||
            GB_aliased (C_in_place, A) ||
            GB_aliased (C_in_place, B) ;
        if (C_transpose || C_aliased)
        { 
            can_do_in_place = false ;
        }
    }

    //--------------------------------------------------------------------------
    // burble
    //--------------------------------------------------------------------------

    #if GB_BURBLE
    char *M_str = (M == NULL) ? "" : (Mask_comp ?  "<!M>" : "<M>") ;
    #define GB_PROP_LEN (GB_LEN+128)
    char A_str [GB_PROP_LEN+1] ;
    char B_str [GB_PROP_LEN+1] ;
    snprintf (A_str, GB_PROP_LEN, "A: "GBd"-by-"GBd", %s, "GBd" entries",
        GB_NROWS (A), GB_NCOLS (A), A->type->name, GB_NNZ (A)) ;
    snprintf (B_str, GB_PROP_LEN, "B: "GBd"-by-"GBd", %s, "GBd" entries",
        GB_NROWS (B), GB_NCOLS (B), B->type->name, GB_NNZ (B)) ;
    #endif

    //--------------------------------------------------------------------------
    // typecast A and B when transposing them, if needed
    //--------------------------------------------------------------------------

    bool op_is_first  = semiring->multiply->opcode == GB_FIRST_opcode ;
    bool op_is_second = semiring->multiply->opcode == GB_SECOND_opcode ;
    bool op_is_pair   = semiring->multiply->opcode == GB_PAIR_opcode ;
    bool A_is_pattern = false ;
    bool B_is_pattern = false ;

    GrB_Type atype_required, btype_required ;
    if (flipxy)
    { 
        // A is passed as y, and B as x, in z = mult(x,y)
        A_is_pattern = op_is_first  || op_is_pair ;
        B_is_pattern = op_is_second || op_is_pair ;
        atype_required = A_is_pattern ? A->type : semiring->multiply->ytype ;
        btype_required = B_is_pattern ? B->type : semiring->multiply->xtype ;
    }
    else
    { 
        // A is passed as x, and B as y, in z = mult(x,y)
        A_is_pattern = op_is_second || op_is_pair ;
        B_is_pattern = op_is_first  || op_is_pair ;
        atype_required = A_is_pattern ? A->type : semiring->multiply->xtype ;
        btype_required = B_is_pattern ? B->type : semiring->multiply->ytype ;
    }

    //--------------------------------------------------------------------------
    // select the algorithm
    //--------------------------------------------------------------------------

    // Four cases remain with the swap_rule above.  M may or may not be
    // present.

        //      C<M> = A *B
        //      C<M> = A *B'
        //      C<M> = A'*B
        //      C<M> = (A*B)'

    if (atrans)
    {

        //----------------------------------------------------------------------
        // C<M> = A'*B' or A'*B
        //----------------------------------------------------------------------

        bool B_is_diagonal = GB_is_diagonal (B, Context) ;

        // explicitly transpose B
        if (btrans && !B_is_diagonal)
        {
            // B = B'
            // with the swap_rule as defined above, this case will never occur.
            // The code is left here in case swap_rule changes in the future.
            ASSERT (GB_DEAD_CODE) ;
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            B = BT ;
        }

        //----------------------------------------------------------------------
        // select the method for C<M>=A'*B
        //----------------------------------------------------------------------

        // A'*B is being computed: use the dot product without computing A'
        // or use the saxpy (heap or Gustavson) method

        // If the mask is present, only entries for which M(i,j)=1 are
        // computed, which makes this method very efficient when the mask is
        // very sparse (triangle counting, for example).  Each entry C(i,j) for
        // which M(i,j)=1 is computed via a dot product, C(i,j) =
        // A(:,i)'*B(:,j).  If the mask is not present, the dot-product method
        // is very slow in general, and thus the saxpy method is usually used
        // instead.

        bool do_rowscale = false ;
        bool do_colscale = false ;
        bool do_adotb = false ;

        if (M == NULL && B_is_diagonal)
        { 
            // C = A'*D
            do_colscale = true ;
        }
        else if (M == NULL && GB_is_diagonal (A, Context))
        { 
            // C = D*B
            do_rowscale = true ;
        }
        else if (AxB_method == GxB_DEFAULT)
        {
            // auto selection for A'*B
            if (M != NULL && !Mask_comp)
            { 
                // C<M>=A'*B uses the masked dot product method
                do_adotb = true ;
            }
            else if (A->vdim == 1 || B->vdim == 1)
            { 
                // C=A'*B uses dot product method if C is a 1-by-n or n-by-1
                do_adotb = true ;
            }
            else
            { 
                // when C is a matrix, C=A'*B uses the dot product method if A
                // or B are dense, since the dot product method requires no
                // workspace in that case and can exploit dense vectors of A
                // and/or B.
                do_adotb = GB_is_dense (A) || GB_is_dense (B) ;
            }
        }
        else
        { 
            // user selection for A'*B
            do_adotb = (AxB_method == GxB_AxB_DOT) ;
        }

        //----------------------------------------------------------------------
        // C<M>=A'*B
        //----------------------------------------------------------------------

        if (do_rowscale)
        { 
            // C = D*B
            GBBURBLE ("C%s=A'*B, rowscale ", M_str) ;
            GB_OK (GB_AxB_rowscale (Chandle, A, B, semiring, flipxy, Context)) ;
        }
        else if (do_colscale)
        { 
            // C = A'*D
            GBBURBLE ("C%s=A'*B, colscale (transposed %s) ", M_str, A_str) ;
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_AxB_colscale (Chandle, AT, B, semiring, flipxy, Context));
        }
        else if (do_adotb)
        { 
            // C<M>=A'*B via dot product, or C_in_place<M>+=A'*B if in place
            GBBURBLE ("C%s=A'*B, %sdot_product ", M_str,
                (M != NULL && !Mask_comp) ? "masked_" : "") ;
            GB_OK (GB_AxB_dot (Chandle, (can_do_in_place) ? C_in_place : NULL,
                M, Mask_comp, Mask_struct, A, B, semiring, flipxy,
                mask_applied, done_in_place, Context)) ;
            (*AxB_method_used) = GxB_AxB_DOT ;
        }
        else
        { 
            // C = A'*B via saxpy3: Gustavson + Hash method
            GBBURBLE ("C%s=A'*B, saxpy (transposed %s) ", M_str, A_str) ;
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_AxB_saxpy3 (Chandle, M, Mask_comp, Mask_struct,
                AT, B, semiring, flipxy, mask_applied, AxB_method, Context)) ;
            (*AxB_method_used) = GxB_AxB_SAXPY ;
        }

    }
    else if (btrans)
    {

        //----------------------------------------------------------------------
        // C<M> = A*B'
        //----------------------------------------------------------------------

        if (M == NULL && GB_is_diagonal (B, Context))
        { 
            // C = A*D
            GBBURBLE ("C%s=A*B', colscale ", M_str) ;
            GB_OK (GB_AxB_colscale (Chandle, A, B, semiring, flipxy, Context)) ;
        }
        else if (M == NULL && GB_is_diagonal (A, Context))
        { 
            // C = D*B'
            GBBURBLE ("C%s=A*B', rowscale (transposed %s) ", M_str, B_str) ;
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            GB_OK (GB_AxB_rowscale (Chandle, A, BT, semiring, flipxy, Context));
        }
        else if (AxB_method == GxB_AxB_DOT)
        { 
            // C<M>=A*B' via dot product, or C_in_place<M>+=A*B' if in place
            GBBURBLE ("C%s=A*B', dot_product (transposed %s) (transposed %s) ",
                M_str, A_str, B_str) ;
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            GB_OK (GB_AxB_dot (Chandle, (can_do_in_place) ? C_in_place : NULL,
                M, Mask_comp, Mask_struct, AT, BT, semiring, flipxy,
                mask_applied, done_in_place, Context)) ;
            (*AxB_method_used) = GxB_AxB_DOT ;
        }
        else
        { 
            // C = A*B' via saxpy3: Gustavson + Hash method
            GBBURBLE ("C%s=A*B', saxpy (transposed %s) ", M_str, B_str) ;
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            GB_OK (GB_AxB_saxpy3 (Chandle, M, Mask_comp, Mask_struct,
                A, BT, semiring, flipxy, mask_applied, AxB_method, Context)) ;
            (*AxB_method_used) = GxB_AxB_SAXPY ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C<M> = A*B
        //----------------------------------------------------------------------

        if (M == NULL && GB_is_diagonal (B, Context))
        { 
            // C = A*D, column scale
            GBBURBLE ("C%s=A*B, colscale ", M_str) ;
            GB_OK (GB_AxB_colscale (Chandle, A, B, semiring, flipxy, Context)) ;
        }
        else if (M == NULL && GB_is_diagonal (A, Context))
        { 
            // C = D*B, row scale
            GBBURBLE ("C%s=A*B, rowscale ", M_str) ;
            GB_OK (GB_AxB_rowscale (Chandle, A, B, semiring, flipxy, Context)) ;
        }
        else if (AxB_method == GxB_AxB_DOT)
        { 
            // C<M>=A*B via dot product, or C_in_place<M>+=A*B if in place
            GBBURBLE ("C%s=A*B', dot_product (transposed %s) ", M_str, A_str) ;
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_AxB_dot (Chandle, (can_do_in_place) ? C_in_place : NULL,
                M, Mask_comp, Mask_struct, AT, B, semiring, flipxy,
                mask_applied, done_in_place, Context)) ;
            (*AxB_method_used) = GxB_AxB_DOT ;
        }
        else
        { 
            // C = A*B via saxpy3: Gustavson + Hash method
            GBBURBLE ("C%s=A*B, saxpy ", M_str) ;
            GB_OK (GB_AxB_saxpy3 (Chandle, M, Mask_comp, Mask_struct,
                A, B, semiring, flipxy, mask_applied, AxB_method, Context)) ;
            (*AxB_method_used) = GxB_AxB_SAXPY ;
        }
    }

    if (M_transposed) { GBBURBLE ("(M transposed) ") ; }
    if ((M != NULL) && !(*mask_applied)) { GBBURBLE ("(mask later) ") ; }

    //--------------------------------------------------------------------------
    // handle C_transpose and assign the CSR/CSC format
    //--------------------------------------------------------------------------

    // If C_transpose is true, then C' has been computed.  In this case, negate
    // the desired C_is_csc so that GB_accum_mask transposes the result before
    // applying the accum operator and/or writing the result back to the user's
    // C.

    if (*done_in_place)
    { 
        // C can be done in place only if C is not transposed on output
        ASSERT_MATRIX_OK (C_in_place, "C_in_place output for all C=A*B", GB0) ;
        ASSERT (C_in_place->is_csc == C_is_csc) ;
    }
    else
    { 
        GrB_Matrix C = (*Chandle) ;
        ASSERT (C != NULL) ;
        C->is_csc = C_transpose ? !C_is_csc : C_is_csc ;
        ASSERT_MATRIX_OK (C, "C output for all C=A*B", GB0) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&BT) ;
    ASSERT_MATRIX_OK_OR_NULL (MT, "MT if computed", GB0) ;
    if (MT_handle != NULL)
    { 
        // return MT to the caller, if computed and the caller wants it
        (*MT_handle) = MT ;
    }
    else
    { 
        // otherwise, free it
        GB_MATRIX_FREE (&MT) ;
    }

    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GB_AxB_meta: C<M>=A*B meta algorithm
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C, C<M>, C<!M> = A*B, A'*B, A*B', or A'*B' : both symbolic and numeric, with
// the optional mask matrix.  This function is called by GB_mxm only.  If the
// mask matrix is present, it can be regular or complemented, and either valued
// or structural.

// This algorithm may decide that it is more efficient to apply the mask later,
// in GB_accum_mask, after this matrix C is computed, in GB_mxm.  The result is
// either the T matrix in GB_mxm, or (if done in-place), the final output
// matrix C passed in from the user (C_in).

// The method is chosen automatically:  a gather/scatter saxpy method
// (Gustavson), or a dot product method.

// FUTURE:: an outer-product method for C=A*B'

#define GB_FREE_ALL             \
{                               \
    GB_Matrix_free (Chandle) ;  \
    GB_Matrix_free (&AT) ;      \
    GB_Matrix_free (&BT) ;      \
    GB_Matrix_free (&MT) ;      \
}

#include "GB_mxm.h"
#include "GB_transpose.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_AxB_meta                // C<M>=A*B meta algorithm
(
    GrB_Matrix *Chandle,            // output matrix (if not done in-place)
    GrB_Matrix C_in,                // input/output matrix, if done in-place
    bool C_replace,                 // C matrix descriptor
    const bool C_is_csc,            // desired CSR/CSC format of C
    GrB_Matrix *MT_handle,          // return MT = M' to caller, if computed
    const GrB_Matrix M_in,          // mask for C<M> (not complemented)
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // accum operator for C_in += A*B
    const GrB_Matrix A_in,          // input matrix
    const GrB_Matrix B_in,          // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    bool A_transpose,               // if true, use A', else A
    bool B_transpose,               // if true, use B', else B
    bool flipxy,                    // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    bool *done_in_place,            // if true, C was computed in-place
    GrB_Desc_Value AxB_method,      // for auto vs user selection of methods
    const int do_sort,              // if nonzero, try to return C unjumbled
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK_OR_NULL (C_in, "C_in for meta A*B", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M_in, "M for meta A*B", GB0) ;
    ASSERT_MATRIX_OK (A_in, "A_in for meta A*B", GB0) ;
    ASSERT_MATRIX_OK (B_in, "B_in for meta A*B", GB0) ;

    ASSERT (!GB_ZOMBIES (M_in)) ;
    ASSERT (GB_JUMBLED_OK (M_in)) ;
    ASSERT (!GB_PENDING (M_in)) ;

    ASSERT (!GB_ZOMBIES (A_in)) ;
    ASSERT (GB_JUMBLED_OK (A_in)) ;
    ASSERT (!GB_PENDING (A_in)) ;

    ASSERT (!GB_ZOMBIES (B_in)) ;
    ASSERT (GB_JUMBLED_OK (B_in)) ;
    ASSERT (!GB_PENDING (B_in)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for numeric A*B", GB0) ;
    ASSERT (mask_applied != NULL) ;
    ASSERT (Chandle != NULL) ;

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------
    // see if the work can be done in-place
    //--------------------------------------------------------------------------

    // If C is hypersparse, sparse, or full:
    //
    //      C can be computed in-place if it is already dense, and if it is
    //      guaranteed to remain dense after the computation is done.  This
    //      case requires the accum operator to be present and it must match
    //      the monoid of the semiring.  C_replace must be false, or
    //      effectively false.
    //
    //      TODO:  if C is full and accum is not present, it can be quickly
    //      converted to bitmap and then done in-place.
    //
    // If C is bitmap:
    //
    //      C can be computed in-place if its type is the same as the semiring
    //      monoid.  The accum must not be present, or if present it must match
    //      the semiring monoid.  C_replace can be true or false.
    //
    //      TODO: modify GB_AxB_dot2 so it can compute C in-place,
    //      or add a bitmap dot product method.  Also modify GB_AxB_saxpy
    //      so it can compute a C bitmap in-place.
    //
    // In both cases, C must not be transposed, nor can it be aliased with any
    // input matrix.

    bool can_do_in_place = false ;

    if (C_in != NULL)
    { 
        if (GB_IS_BITMAP (C_in))
        {
            // C is bitmap
            ASSERT (!GB_PENDING (C_in)) ; // no pending tuples in bitmap
            ASSERT (!GB_ZOMBIES (C_in)) ; // bitmap never has zombies
            can_do_in_place = (C_in->type == semiring->add->op->ztype)
                && ((accum == NULL) || (accum == semiring->add->op)) ;
        }
        else if (accum != NULL)
        { 
            // C is hypersparse, sparse, or full, and accum is present.
            // check if C_in is competely dense:  no pending work.
            bool C_is_dense = GB_as_if_full (C_in) ;

            // accum must be present, and must match the monoid of the
            // semiring, and the ztype of the monoid must match the type of C
            bool accum_is_monoid = (accum == semiring->add->op) 
                && (C_in->type == accum->ztype) ;

            // C += A*B with C_replace ignored (effectively false)
            // C<M> += A*B with C_replace false
            // C<!M> += A*B with C_replace false
            can_do_in_place = C_is_dense && accum_is_monoid
                && ((M_in == NULL) || (M_in != NULL && !C_replace)) ;
        }

        // C must also not be transposed on output; see below.  Nor can it be
        // aliased with any input matrix.  This test is done after handling the
        // CSR/CSC formats since the input matrices may be transposed (thus
        // breaking the alias with C).
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
    // transpose: typecast, no op, not in-place

    GrB_Matrix M ;
    bool M_transposed ;

    // TODO: if Mask_struct is true, do not create values of MT = M'

    if (M_transpose && M_in != NULL)
    { 
        // MT = M_in' also typecasting to boolean.  It is not freed here
        // unless an error occurs, but is returned to the caller.
        GBURBLE ("(M transpose) ") ;
        GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M_in,
            NULL, NULL, NULL, false, Context)) ;
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
        // C cannot be done in-place if it is aliased with any input matrix.
        // Also cannot compute C in-place if it is to be transposed.
        bool C_aliased = GB_aliased (C_in, M) || GB_aliased (C_in, A) ||
            GB_aliased (C_in, B) ;
        if (C_transpose || C_aliased)
        { 
            can_do_in_place = false ;
        }

        // TODO: A and B can be transposed below, so this check should be
        // done after any such transposings.
    }

    //--------------------------------------------------------------------------
    // burble
    //--------------------------------------------------------------------------

    #if GB_BURBLE
    const char *M_str = (M == NULL) ? "" : (Mask_comp ?  "<!M>" : "<M>") ;
    #define GB_PROP_LEN (GB_LEN+128)
    char A_str [GB_PROP_LEN+1] ;
    char B_str [GB_PROP_LEN+1] ;
    if (GB_Global_burble_get ( ))
    {
        int64_t anz = GB_IS_FULL (A) ? GB_NNZ_FULL (A) : GB_NNZ (A) ;
        int64_t bnz = GB_IS_FULL (B) ? GB_NNZ_FULL (B) : GB_NNZ (B) ;
        snprintf (A_str, GB_PROP_LEN, "A: " GBd "-by-" GBd ", %s, " GBd 
            " entries", GB_NROWS (A), GB_NCOLS (A), A->type->name, anz) ;
        snprintf (B_str, GB_PROP_LEN, "B: " GBd "-by-" GBd ", %s, " GBd
            " entries", GB_NROWS (B), GB_NCOLS (B), B->type->name, bnz) ;
    }
    #endif

    //--------------------------------------------------------------------------
    // typecast A and B when transposing them, if needed
    //--------------------------------------------------------------------------

    // TODO: if A is pattern, do not compute values of AT = A'
    // TODO: if B is pattern, do not compute values of BT = B'
    // use constant-valued matrices for AT and BT, with type GrB_BOOL.

    GB_Opcode opcode = semiring->multiply->opcode  ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    bool op_is_first  = (opcode == GB_FIRST_opcode) ;
    bool op_is_second = (opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = (opcode == GB_PAIR_opcode) ;
    bool A_is_pattern ;
    bool B_is_pattern ;

    GrB_Type atype_required, btype_required ;
    if (flipxy)
    { 
        // A is passed as y, and B as x, in z = mult(x,y)
        A_is_pattern = op_is_first  || op_is_pair || op_is_positional ;
        B_is_pattern = op_is_second || op_is_pair || op_is_positional ;
        atype_required = A_is_pattern ? A->type : semiring->multiply->ytype ;
        btype_required = B_is_pattern ? B->type : semiring->multiply->xtype ;
    }
    else
    { 
        // A is passed as x, and B as y, in z = mult(x,y)
        A_is_pattern = op_is_second || op_is_pair || op_is_positional ;
        B_is_pattern = op_is_first  || op_is_pair || op_is_positional ;
        atype_required = A_is_pattern ? A->type : semiring->multiply->xtype ;
        btype_required = B_is_pattern ? B->type : semiring->multiply->ytype ;
    }

    bool allow_scale = true ;
    if (semiring->multiply->function == NULL && (op_is_first || op_is_second))
    { 
        // GB_AxB_rowscale and GB_AxB_colscale do not handle the implicit FIRST
        // operator for GB_reduce_to_vector.  They do handle any other
        // positional operator (FIRSTI, FIRSTJ, SECONDI, SECONDJ, etc).
        allow_scale = false ;
    }

    //--------------------------------------------------------------------------
    // select the algorithm
    //--------------------------------------------------------------------------

    // Four cases remain with the swap_rule.  M may or may not be present.

        //      C<M> = A*B
        //      C<M> = A*B'
        //      C<M> = A'*B
        //      C<M> = (A*B)'

    // use GB_AxB_saxpy3 by default
    #define GB_USE_ROWSCALE 0
    #define GB_USE_COLSCALE 1
    #define GB_USE_DOT      2
    #define GB_USE_SAXPY    3
    int axb_method = GB_USE_SAXPY ;

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
            GB_OK (GB_transpose (&BT, btype_required, true, B,
                NULL, NULL, NULL, false, Context)) ;
            B = BT ;
        }

        //----------------------------------------------------------------------
        // select the method for C<M>=A'*B
        //----------------------------------------------------------------------

        // A'*B is being computed: use the dot product without computing A'
        // or use the saxpy (Gustavson) method

        // If the mask is present, only entries for which M(i,j)=1 are
        // computed, which makes this method very efficient when the mask is
        // very sparse (triangle counting, for example).  Each entry C(i,j) for
        // which M(i,j)=1 is computed via a dot product, C(i,j) =
        // A(:,i)'*B(:,j).  If the mask is not present, the dot-product method
        // is very slow in general, and thus the saxpy method is usually used
        // instead.

        if (allow_scale && M == NULL
            && !GB_IS_BITMAP (A)     // TODO: A'*D colscale with A bitmap
            && B_is_diagonal)
        { 
            // C = A'*D, col scale
            axb_method = GB_USE_COLSCALE ;
        }
        else if (allow_scale && M == NULL
            && !GB_IS_BITMAP (B)     // TODO: D*B rowscale with B bitmap
            && GB_is_diagonal (A, Context))
        { 
            // C = D*B, row scale
            axb_method = GB_USE_ROWSCALE ;
        }
        else if (AxB_method == GxB_DEFAULT)
        {
            // auto selection for A'*B
            if (GB_AxB_dot4_control (can_do_in_place ? C_in : NULL,
                M, Mask_comp))
            {
                // C+=A'*B can be done with dot4
                axb_method = GB_USE_DOT ;
            }
            else if (GB_AxB_dot3_control (M, Mask_comp))
            { 
                // C<M>=A'*B uses the masked dot product method (dot3)
                axb_method = GB_USE_DOT ;
            }
            else if (GB_AxB_dot2_control (A, B, Context))
            {
                // C=A'*B or C<!M>=A'B* can efficiently use the dot2 method
                axb_method = GB_USE_DOT ;
            }

        }
        else if (AxB_method == GxB_AxB_DOT)
        { 
            // user selection for A'*B
            axb_method = GB_USE_DOT ;
        }

        //----------------------------------------------------------------------
        // AT = A'
        //----------------------------------------------------------------------

        if (axb_method == GB_USE_COLSCALE || axb_method == GB_USE_SAXPY)
        {
            // AT = A'
            GB_OK (GB_transpose (&AT, atype_required, true, A,
                NULL, NULL, NULL, false, Context)) ;
            // do not use colscale if AT is now bitmap
            if (GB_IS_BITMAP (AT))
            { 
                axb_method = GB_USE_SAXPY ;
            }
        }

        //----------------------------------------------------------------------
        // C<M>=A'*B
        //----------------------------------------------------------------------

        switch (axb_method)
        {
            case GB_USE_ROWSCALE : 
                // C = D*B using rowscale
                GBURBLE ("C%s=A'*B, rowscale ", M_str) ;
                GB_OK (GB_AxB_rowscale (Chandle, A, B, semiring, flipxy,
                    Context)) ;
                break ;

            case GB_USE_COLSCALE : 
                // C = A'*D using colscale
                GBURBLE ("C%s=A'*B, colscale (transposed %s) ", M_str, A_str) ;
                GB_OK (GB_AxB_colscale (Chandle, AT, B, semiring, flipxy,
                    Context)) ;
                break ;

            case GB_USE_DOT : 
                // C<M>=A'*B via dot, or C_in<M>+=A'*B if in-place
                GBURBLE ("C%s=A'*B, %sdot_product ", M_str,
                    (M != NULL && !Mask_comp) ? "masked_" : "") ;
                GB_OK (GB_AxB_dot (Chandle, (can_do_in_place) ? C_in : NULL,
                    M, Mask_comp, Mask_struct, A, B, semiring, flipxy,
                    mask_applied, done_in_place, Context)) ;
                break ;

            default : 
                // C = A'*B via saxpy: Gustavson + Hash method
                GBURBLE ("C%s=A'*B, saxpy (transposed %s) ", M_str, A_str) ;
                GB_OK (GB_AxB_saxpy (Chandle, M, Mask_comp, Mask_struct,
                    AT, B, semiring, flipxy, mask_applied, AxB_method, do_sort,
                    Context)) ;
                break ;
        }

    }
    else if (btrans)
    {

        //----------------------------------------------------------------------
        // select the method for C<M> = A*B'
        //----------------------------------------------------------------------

        if (allow_scale && M == NULL
            && !GB_IS_BITMAP (A)     // TODO: A*D colscale with A bitmap
            && GB_is_diagonal (B, Context))
        { 
            // C = A*D, column scale
            axb_method = GB_USE_COLSCALE ;
        }
        else if (allow_scale && M == NULL
            && !GB_IS_BITMAP (B)     // TODO: D*B' rowscale with B bitmap
            && GB_is_diagonal (A, Context))
        { 
            // C = D*B', row scale
            axb_method = GB_USE_ROWSCALE ;
        }
        else if (AxB_method == GxB_AxB_DOT)
        { 
            // only use the dot product method if explicitly requested
            axb_method = GB_USE_DOT ;
        }

        //----------------------------------------------------------------------
        // BT = B'
        //----------------------------------------------------------------------

        if (axb_method != GB_USE_COLSCALE)
        {
            // BT = B'
            GB_OK (GB_transpose (&BT, btype_required, true, B,
                NULL, NULL, NULL, false, Context)) ;
            // do not use rowscale if BT is now bitmap
            if (axb_method == GB_USE_ROWSCALE && GB_IS_BITMAP (BT))
            { 
                axb_method = GB_USE_SAXPY ;
            }
        }

        //----------------------------------------------------------------------
        // C<M> = A*B'
        //----------------------------------------------------------------------

        switch (axb_method)
        {
            case GB_USE_COLSCALE : 
                // C = A*D
                GBURBLE ("C%s=A*B', colscale ", M_str) ;
                GB_OK (GB_AxB_colscale (Chandle, A, B, semiring, flipxy,
                    Context)) ;
                break ;

            case GB_USE_ROWSCALE : 
                // C = D*B'
                GBURBLE ("C%s=A*B', rowscale (transposed %s) ", M_str, B_str) ;
                GB_OK (GB_AxB_rowscale (Chandle, A, BT, semiring, flipxy,
                    Context)) ;
                break ;

            case GB_USE_DOT : 
                // C<M>=A*B' via dot product, or C_in<M>+=A*B' if in-place
                GBURBLE ("C%s=A*B', dot_product (transposed %s) "
                    "(transposed %s) ", M_str, A_str, B_str) ;
                GB_OK (GB_transpose (&AT, atype_required, true, A,
                    NULL, NULL, NULL, false, Context)) ;
                GB_OK (GB_AxB_dot (Chandle, (can_do_in_place) ? C_in : NULL,
                    M, Mask_comp, Mask_struct, AT, BT, semiring, flipxy,
                    mask_applied, done_in_place, Context)) ;
                break ;

            default : 
                // C = A*B' via saxpy: Gustavson + Hash method
                GBURBLE ("C%s=A*B', saxpy (transposed %s) ", M_str, B_str) ;
                GB_OK (GB_AxB_saxpy (Chandle, M, Mask_comp, Mask_struct,
                    A, BT, semiring, flipxy, mask_applied, AxB_method, do_sort,
                    Context)) ;
                break ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C<M> = A*B
        //----------------------------------------------------------------------

        if (allow_scale && M == NULL
            && !GB_IS_BITMAP (A)     // TODO: A*D colscale with A bitmap
            && GB_is_diagonal (B, Context))
        { 
            // C = A*D, column scale
            GBURBLE ("C%s=A*B, colscale ", M_str) ;
            GB_OK (GB_AxB_colscale (Chandle, A, B, semiring, flipxy, Context)) ;
        }
        else if (allow_scale && M == NULL
            && !GB_IS_BITMAP (B)     // TODO: D*B rowscale with B bitmap
            && GB_is_diagonal (A, Context))
        { 
            // C = D*B, row scale
            GBURBLE ("C%s=A*B, rowscale ", M_str) ;
            GB_OK (GB_AxB_rowscale (Chandle, A, B, semiring, flipxy, Context)) ;
        }
        else if (AxB_method == GxB_AxB_DOT)
        { 
            // C<M>=A*B via dot product, or C_in<M>+=A*B if in-place.
            // only use the dot product method if explicitly requested
            GBURBLE ("C%s=A*B', dot_product (transposed %s) ", M_str, A_str) ;
            GB_OK (GB_transpose (&AT, atype_required, true, A,
                NULL, NULL, NULL, false, Context)) ;
            GB_OK (GB_AxB_dot (Chandle, (can_do_in_place) ? C_in : NULL,
                M, Mask_comp, Mask_struct, AT, B, semiring, flipxy,
                mask_applied, done_in_place, Context)) ;
        }
        else
        { 
            // C = A*B via saxpy: Gustavson + Hash method
            GBURBLE ("C%s=A*B, saxpy ", M_str) ;
            GB_OK (GB_AxB_saxpy (Chandle, M, Mask_comp, Mask_struct,
                A, B, semiring, flipxy, mask_applied, AxB_method, do_sort,
                Context)) ;
        }
    }

    if (M_transposed) { GBURBLE ("(M transposed) ") ; }
    if ((M != NULL) && !(*mask_applied)) { GBURBLE ("(mask later) ") ; }

    //--------------------------------------------------------------------------
    // handle C_transpose and assign the CSR/CSC format
    //--------------------------------------------------------------------------

    // If C_transpose is true, then C' has been computed.  In this case, negate
    // the desired C_is_csc so that GB_accum_mask transposes the result before
    // applying the accum operator and/or writing the result back to the user's
    // C.

    if (*done_in_place)
    { 
        GBURBLE ("(C in place) ") ;
        // C can be done in-place only if C is not transposed on output
        ASSERT_MATRIX_OK (C_in, "C_in output for all C=A*B", GB0) ;
        ASSERT (C_in->is_csc == C_is_csc) ;
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

    GB_Matrix_free (&AT) ;
    GB_Matrix_free (&BT) ;
    ASSERT_MATRIX_OK_OR_NULL (MT, "MT if computed", GB0) ;
    if (MT_handle != NULL)
    { 
        // return MT to the caller, if computed and the caller wants it
        (*MT_handle) = MT ;
    }
    else
    { 
        // otherwise, free it
        GB_Matrix_free (&MT) ;
    }

    return (GrB_SUCCESS) ;
}


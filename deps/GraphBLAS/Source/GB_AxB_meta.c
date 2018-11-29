//------------------------------------------------------------------------------
// GB_AxB_meta: C<M>=A*B meta algorithm
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C or C<M> = A*B, A'*B, A*B', or A'*B' : both symbolic and numeric, with the
// optional mask matrix.  This function is called by GB_mxm only.  If the mask
// matrix is present, it is not complemented, since this function can only
// handle a non-complemented mask matrix.  A complemented mask is handled in
// GB_accum_mask, after this matrix C is computed.

// The method is chosen automatically:  a gather/scatter saxpy method
// (Gustavson), a heap-based saxpy method, or a dot product method.

// FUTURE: an outer-product method for C=A*B'

#undef GB_OK
#define GB_OK(method)               \
    info = method ;                 \
    if (info != GrB_SUCCESS)        \
    {                               \
        GB_MATRIX_FREE (Chandle) ;  \
        GB_MATRIX_FREE (&AT) ;      \
        GB_MATRIX_FREE (&BT) ;      \
        GB_MATRIX_FREE (&MT) ;      \
        return (info) ;             \
    }

#include "GB.h"

GrB_Info GB_AxB_meta                // C<M>=A*B meta algorithm
(
    GrB_Matrix *Chandle,            // output matrix C
    const bool C_is_csc,            // desired CSR/CSC format of C
    GrB_Matrix *MT_handle,          // return MT = M' to caller, if computed
    const GrB_Matrix M_in,          // mask for C<M> (not complemented)
    const GrB_Matrix A_in,          // input matrix
    const GrB_Matrix B_in,          // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    bool A_transpose,               // if true, use A', else A
    bool B_transpose,               // if true, use B', else B
    bool flipxy,                    // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected
    GB_Sauna *Sauna_Handle,         // handle to sparse accumulator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_NULL (GB_check (M_in, "M for meta A*B", GB0)) ;
    ASSERT_OK (GB_check (A_in, "A_in for meta A*B", GB0)) ;
    ASSERT_OK (GB_check (B_in, "B_in for meta A*B", GB0)) ;
    ASSERT (!GB_PENDING (M_in)) ; ASSERT (!GB_ZOMBIES (M_in)) ;
    ASSERT (!GB_PENDING (A_in)) ; ASSERT (!GB_ZOMBIES (A_in)) ;
    ASSERT (!GB_PENDING (B_in)) ; ASSERT (!GB_ZOMBIES (B_in)) ;
    ASSERT_OK (GB_check (semiring, "semiring for numeric A*B", GB0)) ;
    ASSERT (mask_applied != NULL) ;
    ASSERT (AxB_method_used != NULL) ;
    ASSERT (Sauna_Handle != NULL) ;

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
    (*AxB_method_used) = GxB_DEFAULT ;

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
    // swap_rule: remove the tranpose of C
    //--------------------------------------------------------------------------

    // It is also possible to compute and return C' from this function, and to
    // set C->is_csc as the negation of the desired format C_is_csc.  This
    // ensures that GB_accum_mask will transpose C when this function is done.

    // FUTURE: give user control over the swap_rule.  Any test here will work,
    // with no other changes to the code below.  The decision will only affect
    // the performance, not the result.

    bool swap_rule = (C_transpose) ;

    GrB_Matrix A, B ;
    bool atrans, btrans ;

    if (swap_rule)
    { 
        // Replace C'=A*B with C=B'*A', and so on.  Swap A and B and transose
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

    // Assuming the swap_rule == C_transpose, C no longer needs to be
    // transposed, but the following assertion only holds if swap_rule ==
    // C_transpose.
    ASSERT (!C_transpose) ;

    ASSERT_OK (GB_check (A, "final A for A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "final B for A*B", GB0)) ;

    //--------------------------------------------------------------------------
    // explicitly transpose the mask
    //--------------------------------------------------------------------------

    // all uses of GB_transpose below:
    // transpose: typecast, no op, not in place

    GrB_Matrix M ;

    if (M_transpose && M_in != NULL)
    { 
        // MT = M_in' also typecasting to boolean.  It is not freed here
        // unless an error occurs, but is returned to the caller.
        GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M_in, NULL, Context)) ;
        M = MT ;
    }
    else
    { 
        // M_in can be used as-is; it may be NULL
        M = M_in ;
    }

    ASSERT_OK_OR_NULL (GB_check (M, "final M for A*B", GB0)) ;

    //--------------------------------------------------------------------------
    // typecast A and B when transposing them, if needed
    //--------------------------------------------------------------------------

    GrB_Type atype_required, btype_required ;
    if (flipxy)
    { 
        // A is passed as y, and B as x, in z = mult(x,y)
        atype_required = semiring->multiply->ytype ;
        btype_required = semiring->multiply->xtype ;
    }
    else
    { 
        // A is passed as x, and B as y, in z = mult(x,y)
        atype_required = semiring->multiply->xtype ;
        btype_required = semiring->multiply->ytype ;
    }

    //--------------------------------------------------------------------------
    // select the algorithm
    //--------------------------------------------------------------------------

    if (atrans)
    {

        //----------------------------------------------------------------------
        // C<M> = A'*B' or A'*B
        //----------------------------------------------------------------------

        // explicitly transpose B
        if (btrans)
        {
            // B = B'
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            B = BT ;
        }

        //----------------------------------------------------------------------
        // C<M> = A'*B
        //----------------------------------------------------------------------

        // A'*B is being computed: use the dot product without computing A'
        // or use the saxpy (heap or gather/scatter) method

        // If the mask is present, only entries for which M(i,j)=1 are
        // computed, which makes this method very efficient when the mask is
        // very sparse (triangle counting, for example).  Each entry C(i,j) for
        // which M(i,j)=1 is computed via a dot product, C(i,j) =
        // A(:,i)'*B(:,j).  If the mask is not present, the dot-product method
        // is very slow in general, and thus the saxpy method is usually used
        // instead (via gather/scatter or heap).

        bool use_adotb ;

        if (AxB_method == GxB_DEFAULT)
        {
            // auto selection for A'*B
            if (M != NULL)
            { 
                // C<M> = A'*B always uses the dot product method
                use_adotb = true ;
            }
            else if (A->vdim == 1 || B->vdim == 1)
            { 
                // C=A'*B uses dot product method if C is a 1-by-n or n-by-1
                use_adotb = true ;
            }
            else
            { 
                // when C is a matrix, C=A'*B uses the dot product method if A
                // or B are dense, since the dot product method requires no
                // workspace in that case and can exploit dense vectors of A
                // and/or B.
                GrB_Index bnzmax, anzmax ;
                bool A_is_dense = GB_Index_multiply (&anzmax, A->vlen, A->vdim)
                                  && (anzmax == GB_NNZ (A)) ;
                bool B_is_dense = GB_Index_multiply (&bnzmax, B->vlen, B->vdim)
                                  && (bnzmax == GB_NNZ (B)) ;
                use_adotb = A_is_dense || B_is_dense ;
            }
        }
        else
        { 
            // user selection for A'*B
            use_adotb = (AxB_method == GxB_AxB_DOT) ;
        }

        if (use_adotb)
        { 
            // C<M> = A'*B via dot product method
            (*AxB_method_used) = GxB_AxB_DOT ;
            GB_OK (GB_AxB_dot (Chandle, M, A, B, semiring, flipxy, Context)) ;
        }
        else
        { 
            // C<M> = A'*B via saxpy: gather/scatter or heap method
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_AxB_saxpy (Chandle, M, AT, B, semiring, flipxy,
                AxB_method, AxB_method_used, Sauna_Handle, Context)) ;
        }

    }
    else if (btrans)
    {

        //----------------------------------------------------------------------
        // C<M> = A*B'
        //----------------------------------------------------------------------

        if (AxB_method == GxB_AxB_DOT)
        { 
            // C<M> = A*B' via dot product
            (*AxB_method_used) = GxB_AxB_DOT ;
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            GB_OK (GB_AxB_dot (Chandle, M, AT, BT, semiring, flipxy, Context)) ;
        }
        else
        { 
            // C<M> = A*B' via saxpy: gather/scatter or heap method
            GB_OK (GB_transpose (&BT, btype_required, true, B, NULL, Context)) ;
            GB_OK (GB_AxB_saxpy (Chandle, M, A, BT, semiring, flipxy,
                AxB_method, AxB_method_used, Sauna_Handle, Context)) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C<M> = A*B
        //----------------------------------------------------------------------

        if (AxB_method == GxB_AxB_DOT)
        { 
            // C<M> = A*B via dot product
            (*AxB_method_used) = GxB_AxB_DOT ;
            GB_OK (GB_transpose (&AT, atype_required, true, A, NULL, Context)) ;
            GB_OK (GB_AxB_dot (Chandle, M, AT, B, semiring, flipxy, Context)) ;
        }
        else
        { 
            // C<M> = A*B via saxpy: gather/scatter or heap method
            GB_OK (GB_AxB_saxpy (Chandle, M, A, B, semiring, flipxy,
                AxB_method, AxB_method_used, Sauna_Handle, Context)) ;
        }
    }

    //--------------------------------------------------------------------------
    // handle C_transpose and assign the CSR/CSC format
    //--------------------------------------------------------------------------

    // If C_transpose is true, then C' has been computed.  In this case, negate
    // the desired C_is_csc so that GB_accum_mask transposes the result before
    // applying the accum operator and/or writing the result back to the user's
    // C.  If swap_rule == C_transpose, then C_transpose is always false here,
    // but this could change in the future.  The following code will adapt to
    // any swap_rule, so it does not change if the swap_rule changes.

    GrB_Matrix C = (*Chandle) ;
    ASSERT (C != NULL) ;
    C->is_csc = C_transpose ? !C_is_csc : C_is_csc ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&BT) ;
    ASSERT_OK (GB_check (C, "C output for all C=A*B", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (MT, "MT if computed", GB0)) ;

    (*mask_applied) = (M != NULL) ;
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


//------------------------------------------------------------------------------
// GB_Matrix_multiply: symbolic and numeric C=A*B, A'*B, A*B', or A'*B'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A*B, A'*B, A*B', or A'*B' : both symbolic and numeric.
// Not user-callable.  See GrB_mxm instead.

#define FREE_ALL        \
    GrB_free (&AT) ;    \
    GrB_free (&BT) ;    \
    GrB_free (&MT) ;    \
    GrB_free (&CT) ;

#define OK(method)                  \
    info = method ;                 \
    if (info != GrB_SUCCESS)        \
    {                               \
        FREE_ALL ;                  \
        return (info) ;             \
    }

#include "GB.h"

GrB_Info GB_Matrix_multiply         // C = A*B, A'*B, A*B', or A'*B'
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix Mask,          // Mask for C<M> (not complemented)
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool A_transpose,         // if true, use A', else A
    const bool B_transpose,         // if true, use B', else B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied              // if true, Mask was applied
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C need not be initialized, just the column pointers present
    ASSERT (C != NULL && C->p != NULL && !C->p_shallow) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for generic A*B", 0)) ;
    ASSERT_OK (GB_check (A, "A for generic A*B", 0)) ;
    ASSERT_OK (GB_check (B, "B for generic A*B", 0)) ;
    ASSERT (!PENDING (Mask)) ; ASSERT (!ZOMBIES (Mask)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT_OK (GB_Semiring_check (semiring, "semiring for numeric A*B", 0)) ;
    ASSERT (C->type == semiring->add->op->ztype) ;
    ASSERT (mask_applied != NULL) ;

    #ifndef NDEBUG
    // check the dimensions
    int64_t anrows = (A_transpose) ? A->ncols : A->nrows ;
    int64_t ancols = (A_transpose) ? A->nrows : A->ncols ;
    int64_t bnrows = (B_transpose) ? B->ncols : B->nrows ;
    int64_t bncols = (B_transpose) ? B->nrows : B->ncols ;
    ASSERT (ancols == bnrows && C->nrows == anrows && C->ncols == bncols) ;
    if (Mask != NULL)
    {
        ASSERT (Mask->nrows == C->nrows && Mask->ncols == C->ncols) ;
    }
    #endif

    // cast A and B when transposing them, if needed
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

    GrB_Info info ;
    int64_t f = -1 ;
    int64_t flimit = NNZ (A) + NNZ (B) ;

    GrB_Matrix AT = NULL ;
    GrB_Matrix BT = NULL ;
    GrB_Matrix CT = NULL ;
    GrB_Matrix MT = NULL ;

    // workspace require to compute the transposes
    GrB_Index at_workspace = NNZ (A) + A->nrows + A->ncols ;
    GrB_Index bt_workspace = NNZ (B) + B->nrows + B->ncols ;
    GrB_Index mt_workspace = (Mask == NULL) ? 0 : NNZ (Mask) ;

    bool did_mask = false ;

    //--------------------------------------------------------------------------
    // C = (A or A') * (B or B')
    //--------------------------------------------------------------------------


    if (!B_transpose)
    {
        if (!A_transpose)
        {

            //------------------------------------------------------------------
            // C<M> = A*B
            //------------------------------------------------------------------

            // do not use the mask if the flop count is low
            bool flo = GB_AxB_flopcount (A, B, flimit, &f) ;
            GrB_Matrix M = flo ? NULL : Mask ;
            OK (GB_AxB_symbolic (C, M, A, B, false, false, false)) ;
            OK (GB_AxB_numeric  (C, M, A, B, semiring, flipxy, flo)) ;
            did_mask = (M != NULL) ;
            if (did_mask) ASSERT (ZOMBIES_OK (C)) ;

        }
        else
        {

            //------------------------------------------------------------------
            // C<M> = A'*B
            //------------------------------------------------------------------

            bool use_adotb ;
            if (Mask != NULL)
            {
                // C<M> = A'*B always uses the dot product method.  This might
                // not be the fastest method, but if the outer product method
                // is faster, the user can always transpose A first (AT=A') and
                // then compute C<M> = AT*B, which uses the case above.
                use_adotb = true ;
            }
            else if (C->nrows == 1 || C->ncols == 1)
            {
                // C = A'*B uses the dot product method if C is a vector
                use_adotb = true ;
            }
            else
            {
                // when C is a matrix, C = A'*B uses the dot product method if
                // the workspace required for C is much smaller than the
                // workspace for transposing A or B.
                GrB_Index cwork ;
                bool ok = GB_Index_multiply (&cwork, C->nrows, C->ncols) ;
                use_adotb = ok
                    && cwork < IMIN (at_workspace, 4 * bt_workspace) / 10000 ;
            }

            if (use_adotb)
            {

                //--------------------------------------------------------------
                // C<M> = A'*B using dot products, where A' is not computed
                //--------------------------------------------------------------

                // If the mask is present, only entries for which Mask(i,j)=1
                // are computed, which makes this method very efficient when
                // the mask is very sparse (triangle counting, for example).
                // Each entry C(i,j) for which Mask(i,j)=1 is computed via a
                // dot product, C(i,j)=A(:,i)'*B(:,j).  If the mask is not
                // present, the dot-product method is very slow in general, and
                // thus the outer-product method (GB_AxB_symbolic and _numeric,
                // in the two cases below) is used instead, with A or B being
                // explicitly transposed.

                OK (GB_Matrix_AdotB (C, Mask, A, B, semiring, flipxy)) ;
                did_mask = (Mask != NULL) ;

            }
            else if (at_workspace < 4 * bt_workspace)
            {

                //--------------------------------------------------------------
                // C = A'*B, no mask
                //--------------------------------------------------------------

                // AT = A', typecasting to atype_required
                GB_NEW (&AT, atype_required, A->ncols, A->nrows, false, true) ;
                OK (info) ;
                OK (GB_Matrix_transpose (AT, A, NULL, true)) ;

                // C = A'*B
                bool flo = GB_AxB_flopcount (AT, B, flimit, &f) ;
                OK (GB_AxB_symbolic (C, NULL, AT, B, false, false, false)) ;
                OK (GB_AxB_numeric  (C, NULL, AT, B, semiring, flipxy, flo)) ;
                ASSERT (!ZOMBIES (C)) ;

            }
            else
            {

                //--------------------------------------------------------------
                // C = (B'*A)', no mask
                //--------------------------------------------------------------

                // BT = B', typecasting to btype_required
                GB_NEW (&BT, btype_required, B->ncols, B->nrows, false, true) ;
                OK (info) ;
                OK (GB_Matrix_transpose (BT, B, NULL, true)) ;

                // CT = BT*A
                GB_NEW (&CT, C->type, C->ncols, C->nrows, false, true) ;
                OK (info) ;

                bool flo = GB_AxB_flopcount (BT, A, flimit, &f) ;
                OK (GB_AxB_symbolic (CT, NULL, BT, A, false, false, false)) ;
                OK (GB_AxB_numeric  (CT, NULL, BT, A, semiring, !flipxy, flo)) ;
                ASSERT (!ZOMBIES (CT)) ;
                GrB_free (&BT) ;

                // C = CT', no typecasting, no operator
                OK (GB_Matrix_transpose (C, CT, NULL, true)) ;

            }
        }
    }
    else
    {
        if (!A_transpose)
        {

            //------------------------------------------------------------------
            // C<M> = A*B'
            //------------------------------------------------------------------

            // select the method that uses the least workspace
            if (bt_workspace < 4 * (at_workspace + mt_workspace))
            {

                //--------------------------------------------------------------
                // C<M> = A*B'
                //--------------------------------------------------------------

                // BT = B', typecasting to btype_required
                GB_NEW (&BT, btype_required, B->ncols, B->nrows, false, true) ;
                OK (info) ;
                OK (GB_Matrix_transpose (BT, B, NULL, true)) ;

                // C = A*BT
                bool flo = GB_AxB_flopcount (A, BT, flimit, &f) ;
                GrB_Matrix M = flo ? NULL : Mask ;
                OK (GB_AxB_symbolic (C, M, A, BT, false, false, false)) ;
                OK (GB_AxB_numeric  (C, M, A, BT, semiring, flipxy, flo)) ;
                did_mask = (M != NULL) ;
                if (did_mask) ASSERT (ZOMBIES_OK (C)) ;

            }
            else
            {

                //--------------------------------------------------------------
                // C<M> = (B*A')'
                //--------------------------------------------------------------

                // AT = A', typecasting to atype_required
                GB_NEW (&AT, atype_required, A->ncols, A->nrows, false, true) ;
                OK (info) ;
                OK (GB_Matrix_transpose (AT, A, NULL, true)) ;

                if (Mask != NULL)
                {
                    // MT = Mask', typecasting to boolean
                    GB_NEW (&MT, GrB_BOOL, Mask->ncols, Mask->nrows,
                        false, true) ;
                    OK (info) ;
                    OK (GB_Matrix_transpose (MT, Mask, NULL, true)) ;
                }

                // CT = B*AT
                GB_NEW (&CT, C->type, C->ncols, C->nrows, false, true) ;
                OK (info) ;

                bool flo = GB_AxB_flopcount (B, AT, flimit, &f) ;
                GrB_Matrix M = flo ? NULL : MT ;
                OK (GB_AxB_symbolic (CT, M, B, AT, false, false, false)) ;
                OK (GB_AxB_numeric  (CT, M, B, AT, semiring, !flipxy, flo)) ;
                did_mask = (M != NULL) ;
                if (did_mask) ASSERT (ZOMBIES_OK (CT)) ;
                GrB_free (&MT) ;
                GrB_free (&AT) ;

                // C = CT', no typecasting, no operator
                APPLY_PENDING_UPDATES (CT) ;
                OK (GB_Matrix_transpose (C, CT, NULL, true)) ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // C<M> = A'*B' = (B*A)'
            //------------------------------------------------------------------

            // select the method based on the flop count
            GB_NEW (&CT, C->type, C->ncols, C->nrows, false, true) ;
            OK (info) ;

            bool flo = GB_AxB_flopcount (B, A, flimit, &f) ;
            if (flo)
            {

                //--------------------------------------------------------------
                // CT = B*A
                //--------------------------------------------------------------

                // flop count is low, so transposing A and B takes more time
                // than computing B*A.  Do not exploit the Mask, and do not
                // transpose A or B in the symbolic analysis.
                OK (GB_AxB_symbolic (CT, NULL, B, A, false, false, false)) ;
                OK (GB_AxB_numeric  (CT, NULL, B, A, semiring, !flipxy, true)) ;
                ASSERT (!ZOMBIES (CT)) ;

            }
            else
            {

                //--------------------------------------------------------------
                // CT<MT> = B*A
                //--------------------------------------------------------------

                // flop count is high, so exploit the Mask if present.  If the
                // Mask is not present, do a full symbolic analysis of
                // CT=(A'*B')' where the pattern of A and B are transposed and
                // the resulting pattern (A'*B') is transposed.  This avoids
                // the sort of each column of the result.
                if (Mask != NULL)
                {
                    // MT = Mask', typecasting to boolean
                    GB_NEW (&MT, GrB_BOOL, Mask->ncols, Mask->nrows,
                        false, true) ;
                    OK (info) ;
                    OK (GB_Matrix_transpose (MT, Mask, NULL, true)) ;
                }
                OK (GB_AxB_symbolic (CT, MT, A, B, true, true, true)) ;
                // an alternative (same result but without transposing):
                //  GB_AxB_symbolic (CT, MT, B, A, false, false, false)) ;
                OK (GB_AxB_numeric  (CT, MT, B, A, semiring, !flipxy, false)) ;
                did_mask = (MT != NULL) ;
                if (did_mask) ASSERT (ZOMBIES_OK (CT)) ;
                GrB_free (&MT) ;
                APPLY_PENDING_UPDATES (CT) ;
            }

            // C = CT', no typecasting, no operator
            OK (GB_Matrix_transpose (C, CT, NULL, true)) ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    FREE_ALL ;
    ASSERT_OK (GB_check (C, "C output for generic C=A*B", 0)) ;
    (*mask_applied) = did_mask ;
    if (did_mask) { ASSERT (ZOMBIES_OK (C)) ; } else { ASSERT (!ZOMBIES (C)) ; }
    return (REPORT_SUCCESS) ;
}

#undef OK
#undef FREE_ALL


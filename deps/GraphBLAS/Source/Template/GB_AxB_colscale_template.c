//------------------------------------------------------------------------------
// GB_AxB_colscale_template: C=A*D where D is a square diagonal matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This template is not used If C is iso, since all that is needed is to create
// C as a shallow-copy of the pattern of A.

// A and C can be jumbled.  D cannot, but it is a diagonal matrix so it is
// never jumbled.

{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_JUMBLED (D)) ;
    ASSERT (!C->iso) ;

    //--------------------------------------------------------------------------
    // get C, A, and D
    //--------------------------------------------------------------------------

    const int64_t  *restrict Ap = A->p ;
    const int64_t  *restrict Ah = A->h ;
    #if !GB_A_IS_PATTERN
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    #endif
    #if !GB_B_IS_PATTERN
    const GB_BTYPE *restrict Dx = (GB_BTYPE *) D->x ;
    #endif
    const int64_t avlen = A->vlen ;
    const bool A_iso = A->iso ;
    const bool D_iso = D->iso ;

    const int64_t *restrict kfirst_Aslice = A_ek_slicing ;
    const int64_t *restrict klast_Aslice  = A_ek_slicing + A_ntasks ;
    const int64_t *restrict pstart_Aslice = A_ek_slicing + A_ntasks * 2 ;

    //--------------------------------------------------------------------------
    // C=A*D
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(A_nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < A_ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_Aslice [tid] ;
        int64_t klast  = klast_Aslice  [tid] ;

        //----------------------------------------------------------------------
        // C(:,kfirst:klast) = A(:,kfirst:klast)*D(kfirst:klast,kfirst:klast)
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) and C(:,k) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA_start, pA_end ;
            GB_get_pA (&pA_start, &pA_end, tid, k,
                kfirst, klast, pstart_Aslice, Ap, avlen) ;

            //------------------------------------------------------------------
            // C(:,j) = A(:,j)*D(j,j)
            //------------------------------------------------------------------

            GB_GETB (djj, Dx, j, D_iso) ;           // djj = D (j,j)
            GB_PRAGMA_SIMD_VECTORIZE
            for (int64_t p = pA_start ; p < pA_end ; p++)
            { 
                GB_GETA (aij, Ax, p, A_iso) ;           // aij = A(i,j)
                GB_BINOP (GB_CX (p), aij, djj, 0, 0) ;  // C(i,j) = aij * djj
            }
        }
    }
}


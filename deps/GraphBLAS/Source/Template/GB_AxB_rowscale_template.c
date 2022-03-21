//------------------------------------------------------------------------------
// GB_AxB_rowscale_template: C=D*B where D is a square diagonal matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This template is not used If C is iso, since all that is needed is to create
// C as a shallow-copy of the pattern of A.

// B and C can be jumbled.  D cannot, but it is a diagonal matrix so it is
// never jumbled.

{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (!GB_JUMBLED (D)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;
    ASSERT (!C->iso) ;

    //--------------------------------------------------------------------------
    // get D and B
    //--------------------------------------------------------------------------

    #if !GB_A_IS_PATTERN
    const GB_ATYPE *restrict Dx = (GB_ATYPE *) D->x ;
    #endif
    const bool D_iso = D->iso ;
    #if !GB_B_IS_PATTERN
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
    #endif
    const bool B_iso = B->iso ;
    const int64_t *restrict Bi = B->i ;
    const int64_t bnz = GB_nnz (B) ;
    const int64_t bvlen = B->vlen ;

    //--------------------------------------------------------------------------
    // C=D*B
    //--------------------------------------------------------------------------

    int ntasks = nthreads ;
    ntasks = GB_IMIN (bnz, ntasks) ;

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (tid = 0 ; tid < ntasks ; tid++)
    {
        int64_t pstart, pend ;
        GB_PARTITION (pstart, pend, bnz, tid, ntasks) ;
        GB_PRAGMA_SIMD_VECTORIZE
        for (int64_t p = pstart ; p < pend ; p++)
        { 
            int64_t i = GBI (Bi, p, bvlen) ;        // get row index of B(i,j)
            GB_GETA (dii, Dx, i, D_iso) ;           // dii = D(i,i)
            GB_GETB (bij, Bx, p, B_iso) ;           // bij = B(i,j)
            GB_BINOP (GB_CX (p), dii, bij, 0, 0) ;  // C(i,j) = dii*bij
        }
    }
}


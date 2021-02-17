//------------------------------------------------------------------------------
// GB_AxB_rowscale_meta: C=D*B where D is a square diagonal matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All entries in C=D*B are computed entirely in parallel. 

// B and C can be jumbled.  D cannot, but it is a diagonal matrix so it is
// never jumbled.

{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // Bx is unused if the operator is FIRST or PAIR
    #include "GB_unused.h"

    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (!GB_JUMBLED (D)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;

    //--------------------------------------------------------------------------
    // get C, D, and B
    //--------------------------------------------------------------------------

    const GB_ATYPE *GB_RESTRICT Dx = (GB_ATYPE *) (D_is_pattern ? NULL : D->x) ;
    const GB_BTYPE *GB_RESTRICT Bx = (GB_BTYPE *) (B_is_pattern ? NULL : B->x) ;
    const int64_t  *GB_RESTRICT Bi = B->i ;
    const int64_t bnz = GB_IS_FULL (B) ? GB_NNZ_FULL (B) : GB_NNZ (B) ;
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
            GB_GETA (dii, Dx, i) ;                  // dii = D(i,i)
            GB_GETB (bij, Bx, p) ;                  // bij = B(i,j)
            GB_BINOP (GB_CX (p), dii, bij, 0, 0) ;  // C(i,j) = dii*bij
        }
    }
}


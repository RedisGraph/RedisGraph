//------------------------------------------------------------------------------
// GB_AxB_rowscale_meta: C=D*B where D is a square diagonal matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All entries in C=D*B are computed fully in parallel. 

{
    // Bx is unused if the operator is FIRST or PAIR
    #include "GB_unused.h"

    //--------------------------------------------------------------------------
    // get C, D, and B
    //--------------------------------------------------------------------------

    const GB_ATYPE *GB_RESTRICT Dx = D_is_pattern ? NULL : D->x ;
    const GB_BTYPE *GB_RESTRICT Bx = B_is_pattern ? NULL : B->x ;
    const int64_t  *GB_RESTRICT Bi = B->i ;
    int64_t bnz = GB_NNZ (B) ;

    //--------------------------------------------------------------------------
    // C=D*B
    //--------------------------------------------------------------------------

    int ntasks = (nthreads == 1) ? 1 : (32 * nthreads) ;
    ntasks = GB_IMIN (bnz, ntasks) ;

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {
        int64_t pstart, pend ;
        GB_PARTITION (pstart, pend, bnz, tid, ntasks) ;
        GB_PRAGMA_VECTORIZE
        for (int64_t p = pstart ; p < pend ; p++)
        { 
            int64_t i = Bi [p] ;                // get row index of B(i,j)
            GB_GETA (dii, Dx, i) ;              // dii = D(i,i)
            GB_GETB (bij, Bx, p) ;              // bij = B(i,j)
            GB_BINOP (GB_CX (p), dii, bij) ;    // C(i,j) = dii*bij
        }
    }
}


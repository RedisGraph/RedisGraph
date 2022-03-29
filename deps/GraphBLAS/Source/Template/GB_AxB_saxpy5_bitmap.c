//------------------------------------------------------------------------------
// GB_AxB_saxpy5_bitmap.c: C+=A*B when C is full, A is bitmap, B is sparse/hyper
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is as-if-full.
// A is bitmap, and not iso-valued and not pattern-only
// B is sparse or hypersparse.

{

    //--------------------------------------------------------------------------
    // get C, A, and B
    //--------------------------------------------------------------------------

    const int64_t m = C->vlen ;     // # of rows of C and A
    const int8_t  *restrict Ab = A->b ;
    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bh = B->h ;
    const int64_t *restrict Bi = B->i ;
    const bool B_iso = B->iso ;
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    #if !GB_B_IS_PATTERN
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
    #endif
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;

    //--------------------------------------------------------------------------
    // C += A*B where A is bitmap (and not iso or pattern-only)
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {
        // get the task descriptor
        const int64_t jB_start = B_slice [tid] ;
        const int64_t jB_end   = B_slice [tid+1] ;
        // C(:,jB_start:jB_end-1) += A * B(:,jB_start:jB_end-1)
        for (int64_t jB = jB_start ; jB < jB_end ; jB++)
        {
            // get B(:,j) and C(:,j)
            const int64_t j = GBH (Bh, jB) ;
            const int64_t pC = j * m ;
            const int64_t pB_start = Bp [jB] ;
            const int64_t pB_end   = Bp [jB+1] ;
            // C(:,j) += A*B(:,j)
            for (int64_t pB = pB_start ; pB < pB_end ; pB++)
            { 
                // get B(k,j)
                const int64_t k = Bi [pB] ;
                GB_GETB (bkj, Bx, pB, B_iso) ;
                // get A(:,k)
                const int64_t pA = k * m ;
                // C(:,j) += A(:,k)*B(k,j)
                for (int64_t i = 0 ; i < m ; i++)
                { 
                    if (!Ab [pA+i]) continue ;
                    // C(i,j) += A(i,k)*B(k,j) ;
                    GB_MULTADD (Cx [pC + i], Ax [pA + i], bkj, i, k, j) ;
                }
            }
        }
    }
}


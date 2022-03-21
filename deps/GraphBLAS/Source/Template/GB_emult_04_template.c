//------------------------------------------------------------------------------
// GB_emult_04_template: C<M>= A.*B, M sparse/hyper, A and B bitmap/full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is sparse, with the same sparsity structure as M.
// A and B are both bitmap/full.

{

    //--------------------------------------------------------------------------
    // get M, A, B, and C
    //--------------------------------------------------------------------------

    const int8_t *restrict Ab = A->b ;
    const int8_t *restrict Bb = B->b ;
    const bool A_iso = A->iso ;
    const bool B_iso = B->iso ;

    #ifdef GB_ISO_EMULT
    ASSERT (C->iso) ;
    #else
    ASSERT (!C->iso) ;
    ASSERT (!(A_iso && B_iso)) ;    // one of A or B can be iso, but not both
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mh = M->h ;
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = (GB_void *) ((Mask_struct) ? NULL : M->x) ;
    const int64_t vlen = M->vlen ;
    const size_t  msize = M->type->size ;

    const int64_t  *restrict Cp = C->p ;
          int64_t  *restrict Ci = C->i ;

    const int64_t *restrict kfirst_Mslice = M_ek_slicing ;
    const int64_t *restrict klast_Mslice  = M_ek_slicing + M_ntasks ;
    const int64_t *restrict pstart_Mslice = M_ek_slicing + M_ntasks * 2 ;

    //--------------------------------------------------------------------------
    // Method4: C<M>=A.*B where M is sparse/hyper, A and B are bitmap/full
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(M_nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < M_ntasks ; tid++)
    {
        int64_t kfirst = kfirst_Mslice [tid] ;
        int64_t klast  = klast_Mslice  [tid] ;
        for (int64_t k = kfirst ; k <= klast ; k++)
        {
            int64_t j = GBH (Mh, k) ;
            int64_t pstart = j * vlen ;
            int64_t pM, pM_end, pC ;
            GB_get_pA_and_pC (&pM, &pM_end, &pC, tid, k, kfirst, klast,
                pstart_Mslice, Cp_kfirst, Cp, vlen, Mp, vlen) ;
            for ( ; pM < pM_end ; pM++)
            {
                int64_t i = Mi [pM] ;
                if (GB_mcast (Mx, pM, msize) &&
                    (GBB (Ab, pstart + i)
                    &&  // TODO: for GB_add, use || instead
                    GBB (Bb, pstart + i)))
                { 
                    int64_t p = pstart + i ;
                    // C (i,j) = A (i,j) .* B (i,j)
                    Ci [pC] = i ;
                    #ifndef GB_ISO_EMULT
                    GB_GETA (aij, Ax, p, A_iso) ;
                    GB_GETB (bij, Bx, p, B_iso) ;
                    GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                    #endif
                    pC++ ;
                }
            }
        }
    }
}


//------------------------------------------------------------------------------
// GB_select_phase2: C=select(A,thunk)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{
    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) A->x ;
    size_t asize = A->type->size ;
    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;
    // if A is bitmap, the bitmap selector is always used instead
    ASSERT (!GB_IS_BITMAP (A)) ;
    #ifndef GB_DIAG_SELECTOR
    // if A is full, all opcodes except DIAG use the bitmap selector instead
    ASSERT (!GB_IS_FULL (A)) ;
    #endif

    //--------------------------------------------------------------------------
    // C = select (A)
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        //----------------------------------------------------------------------
        // selection from vectors kfirst to klast
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) to be operated on by this task
            //------------------------------------------------------------------

            int64_t pA_start, pA_end, pC ;
            GB_get_pA_and_pC (&pA_start, &pA_end, &pC, tid, k, kfirst, klast,
                pstart_slice, C_pstart_slice, Cp, avlen, Ap, avlen) ;

            //------------------------------------------------------------------
            // compact Ai and Ax [pA_start ... pA_end-1] into Ci and Cx
            //------------------------------------------------------------------

            #if defined ( GB_ENTRY_SELECTOR )

                GB_GET_J ;
                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                {
                    // A is never full; that case is now handled by the
                    // bitmap selector instead.
                    // int64_t i = GBI (Ai, pA, avlen) ;
                    ASSERT (Ai != NULL) ;
                    int64_t i = Ai [pA] ;
                    if (GB_TEST_VALUE_OF_ENTRY (pA))
                    { 
                        ASSERT (pC >= Cp [k] && pC < Cp [k+1]) ;
                        Ci [pC] = i ;
                        // Cx [pC] = Ax [pA] ;
                        GB_SELECT_ENTRY (Cx, pC, Ax, pA) ;
                        pC++ ;
                    }
                }

            #elif defined ( GB_TRIU_SELECTOR ) \
              ||  defined ( GB_RESIZE_SELECTOR )

                // keep pA_start to Zp[k]-1
                int64_t p = GB_IMIN (Zp [k], pA_end) ;
                int64_t mynz = p - pA_start ;
                if (mynz > 0)
                { 
                    ASSERT (pC >= Cp [k] && pC + mynz <= Cp [k+1]) ;
                    ASSERT (Ai != NULL) ;
                    // if (Ai != NULL)
                    {
                        // A and C are both sparse or hypersparse
                        memcpy (Ci +pC, Ai +pA_start, mynz*sizeof (int64_t)) ;
                    }
                    #if 0
                    else
                    {
                        // A is full and C is sparse: for triu: the bitmap
                        // selector is used.  For resize, A is converted to
                        // hypersparse first.
                        ASSERT (GB_DEAD_CODE) ;
                        int64_t i_start = pA_start % avlen ;
                        for (int64_t s = 0 ; s < mynz ; s++)
                        {
                            int64_t i = i_start + s ;
                            ASSERT (GBI (Ai, pA_start+s, avlen) == i) ;
                            Ci [pC+s] = i ;
                        }
                    }
                    #endif
                    memcpy (Cx +pC*asize, Ax +pA_start*asize, mynz*asize) ;
                }

            #elif defined ( GB_DIAG_SELECTOR )

                // task that owns the diagonal entry does this work
                // A can be sparse or full, but not bitmap
                int64_t p = Zp [k] ;
                if (pA_start <= p && p < pA_end)
                { 
                    ASSERT (pC >= Cp [k] && pC + 1 <= Cp [k+1]) ;
                    Ci [pC] = GBI (Ai, p, avlen) ;
                    memcpy (Cx +pC*asize, Ax +p*asize, asize) ;
                }

            #elif defined ( GB_OFFDIAG_SELECTOR )

                // keep pA_start to Zp[k]-1
                int64_t p = GB_IMIN (Zp [k], pA_end) ;
                int64_t mynz = p - pA_start ;
                if (mynz > 0)
                { 
                    ASSERT (pC >= Cp [k] && pC + mynz <= Cp [k+1]) ;
                    ASSERT (Ai != NULL) ;
                    // if (Ai != NULL)
                    {
                        // A and C are both sparse or hypersparse
                        memcpy (Ci +pC, Ai +pA_start, mynz*sizeof (int64_t)) ;
                    }
                    #if 0
                    else
                    {
                        // A is full and C is sparse or hypersparse:
                        // this is now always handled by the bitmap selector
                        ASSERT (GB_DEAD_CODE) ;
                        int64_t i_start = pA_start % avlen ;
                        for (int64_t s = 0 ; s < mynz ; s++)
                        {
                            int64_t i = i_start + s ;
                            ASSERT (GBI (Ai, pA_start+s, avlen) == i) ;
                            Ci [pC+s] = i ;
                        }
                    }
                    #endif
                    memcpy (Cx +pC*asize, Ax +pA_start*asize, mynz*asize) ;
                    pC += mynz ;
                }

                // keep Zp[k]+1 to pA_end-1
                p = GB_IMAX (Zp [k]+1, pA_start) ;
                mynz = pA_end - p ;
                if (mynz > 0)
                { 
                    ASSERT (pA_start <= p && p < pA_end) ;
                    ASSERT (pC >= Cp [k] && pC + mynz <= Cp [k+1]) ;
                    ASSERT (Ai != NULL) ;
                    // if (Ai != NULL)
                    {
                        // A and C are both sparse or hypersparse
                        memcpy (Ci +pC, Ai +p, mynz*sizeof (int64_t)) ;
                    }
                    #if 0
                    else
                    {
                        // A is full and C is sparse or hypersparse
                        ASSERT (GB_DEAD_CODE) ;
                        int64_t i_start = p % avlen ;
                        for (int64_t s = 0 ; s < mynz ; s++)
                        {
                            int64_t i = i_start + s ;
                            ASSERT (GBI (Ai, p+s, avlen) == i) ;
                            Ci [pC+s] = i ;
                        }
                    }
                    #endif
                    memcpy (Cx +pC*asize, Ax +p*asize, mynz*asize) ;
                }

            #elif defined ( GB_TRIL_SELECTOR )

                // keep Zp [k] to pA_end-1
                int64_t p = GB_IMAX (Zp [k], pA_start) ;
                int64_t mynz = pA_end - p ;
                if (mynz > 0)
                { 
                    ASSERT (pA_start <= p && p + mynz <= pA_end) ;
                    ASSERT (pC >= Cp [k] && pC + mynz <= Cp [k+1]) ;
                    ASSERT (Ai != NULL) ;
                    // if (Ai != NULL)
                    {
                        // A and C are both sparse or hypersparse
                        memcpy (Ci +pC, Ai +p, mynz*sizeof (int64_t)) ;
                    }
                    #if 0
                    else
                    {
                        // A is full and C is sparse or hypersparse:
                        // this is now always handled by the bitmap selector
                        ASSERT (GB_DEAD_CODE) ;
                        int64_t i_start = p % avlen ;
                        for (int64_t s = 0 ; s < mynz ; s++)
                        {
                            int64_t i = i_start + s ;
                            ASSERT (GBI (Ai, p+s, avlen) == i) ;
                            Ci [pC+s] = i ;
                        }
                    }
                    #endif
                    memcpy (Cx +pC*asize, Ax +p*asize, mynz*asize) ;
                }

            #endif
        }
    }
}


//------------------------------------------------------------------------------
// GB_select_phase1: count entries in each vector for C=select(A,thunk)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    // get A and its slicing
    //--------------------------------------------------------------------------

    const int64_t *restrict kfirst_Aslice = A_ek_slicing ;
    const int64_t *restrict klast_Aslice  = A_ek_slicing + A_ntasks ;
    const int64_t *restrict pstart_Aslice = A_ek_slicing + A_ntasks * 2 ;

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ai = A->i ;
    int64_t avlen = A->vlen ;
    int64_t anvec = A->nvec ;

#if defined ( GB_ENTRY_SELECTOR )

    //==========================================================================
    // entry selector
    //==========================================================================

    ASSERT (GB_JUMBLED_OK (A)) ;

    // The count of live entries kth vector A(:,k) is reduced to the kth scalar
    // Cp(k).  Each thread computes the reductions on roughly the same number
    // of entries, which means that a vector A(:,k) may be reduced by more than
    // one thread.  The first vector A(:,kfirst) reduced by thread tid may be
    // partial, where the prior thread tid-1 (and other prior threads) may also
    // do some of the reductions for this same vector A(:,kfirst).  The thread
    // tid reduces all vectors A(:,k) for k in the range kfirst+1 to klast-1.
    // The last vector A(:,klast) reduced by thread tid may also be partial.
    // Thread tid+1, and following threads, may also do some of the reduces for
    // A(:,klast).

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    size_t  asize = A->type->size ;
    int64_t avdim = A->vdim ;
    ASSERT (GB_JUMBLED_OK (A)) ;

    //--------------------------------------------------------------------------
    // reduce each slice
    //--------------------------------------------------------------------------

    // each thread reduces its own part in parallel
    int tid ;
    #pragma omp parallel for num_threads(A_nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < A_ntasks ; tid++)
    {

        // if kfirst > klast then thread tid does no work at all
        int64_t kfirst = kfirst_Aslice [tid] ;
        int64_t klast  = klast_Aslice  [tid] ;
        Wfirst [tid] = 0 ;
        Wlast  [tid] = 0 ;

        //----------------------------------------------------------------------
        // reduce vectors kfirst to klast
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) to be reduced by this thread
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA, pA_end ;
            GB_get_pA (&pA, &pA_end, tid, k,
                kfirst, klast, pstart_Aslice, Ap, avlen) ;

            //------------------------------------------------------------------
            // count entries in Ax [pA ... pA_end-1]
            //------------------------------------------------------------------

            int64_t cjnz = 0 ;
            for ( ; pA < pA_end ; pA++)
            { 
                ASSERT (Ai != NULL) ;
                int64_t i = Ai [pA] ;
                GB_TEST_VALUE_OF_ENTRY (keep, pA) ;
                if (keep) cjnz++ ;
            }
            if (k == kfirst)
            { 
                Wfirst [tid] = cjnz ;
            }
            else if (k == klast)
            { 
                Wlast [tid] = cjnz ;
            }
            else
            { 
                Cp [k] = cjnz ; 
            }
        }
    }

    //--------------------------------------------------------------------------
    // reduce the first and last vector of each slice using a single thread
    //--------------------------------------------------------------------------

    GB_ek_slice_merge1 (Cp, Wfirst, Wlast, A_ek_slicing, A_ntasks) ;

#else

    //==========================================================================
    // positional selector (tril, triu, diag, offdiag, resize, row*)
    //==========================================================================

    ASSERT (!GB_JUMBLED (A)) ;

    //--------------------------------------------------------------------------
    // tril, triu, diag, offdiag, resize: binary search in each vector
    //--------------------------------------------------------------------------

    int64_t k ;
    #pragma omp parallel for num_threads(A_nthreads) schedule(guided)
    for (k = 0 ; k < anvec ; k++)
    {

        //----------------------------------------------------------------------
        // get A(:,k)
        //----------------------------------------------------------------------

        int64_t pA_start = GBP (Ap, k, avlen) ;
        int64_t pA_end   = GBP (Ap, k+1, avlen) ;
        int64_t p = pA_start ;
        int64_t cjnz = 0 ;
        int64_t ajnz = pA_end - pA_start ;
        bool found = false ;

        if (ajnz > 0)
        {

            //------------------------------------------------------------------
            // search for the entry A(i,k)
            //------------------------------------------------------------------

            int64_t ifirst = GBI (Ai, pA_start, avlen) ;
            int64_t ilast  = GBI (Ai, pA_end-1, avlen) ;

            #if defined ( GB_ROWINDEX_SELECTOR )
            int64_t i = -ithunk ;
            #elif defined ( GB_ROWLE_SELECTOR ) || defined ( GB_ROWGT_SELECTOR )
            int64_t i = ithunk ;
            #else
            // TRIL, TRIU, DIAG, OFFDIAG
            int64_t j = GBH (Ah, k) ;
            int64_t i = j-ithunk ;
            #endif

            if (i < ifirst)
            { 
                // all entries in A(:,k) come after i
                ;
            }
            else if (i > ilast)
            { 
                // all entries in A(:,k) come before i
                p = pA_end ;
            }
            else if (ajnz == avlen)
            { 
                // A(:,k) is dense
                found = true ;
                p += i ;
                ASSERT (GBI (Ai, p, avlen) == i) ;
            }
            else
            { 
                // binary search for A (i,k)
                int64_t pright = pA_end - 1 ;
                GB_SPLIT_BINARY_SEARCH (i, Ai, p, pright, found) ;
            }

            #if defined ( GB_TRIL_SELECTOR )

                // keep p to pA_end-1
                cjnz = pA_end - p ;

            #elif defined ( GB_ROWGT_SELECTOR  )

                // if found, keep p+1 to pA_end-1
                // else keep p to pA_end-1
                if (found)
                { 
                    p++ ;
                    // now in both cases, keep p to pA_end-1
                }
                // keep p to pA_end-1
                cjnz = pA_end - p ;

            #elif defined ( GB_TRIU_SELECTOR   ) \
               || defined ( GB_ROWLE_SELECTOR  )

                // if found, keep pA_start to p
                // else keep pA_start to p-1
                if (found)
                { 
                    p++ ;
                    // now in both cases, keep pA_start to p-1
                }
                // keep pA_start to p-1
                cjnz = p - pA_start ;

            #elif defined ( GB_DIAG_SELECTOR )

                // if found, keep p
                // else keep nothing
                cjnz = found ;
                if (!found) p = -1 ;
                // if (cjnz >= 0) keep p, else keep nothing

            #elif defined ( GB_OFFDIAG_SELECTOR  ) || \
                  defined ( GB_ROWINDEX_SELECTOR )

                // if found, keep pA_start to p-1 and p+1 to pA_end-1
                // else keep pA_start to pA_end
                cjnz = ajnz - found ;
                if (!found)
                { 
                    p = pA_end ;
                    // now just keep pA_start to p-1; p+1 to pA_end is 
                    // now empty
                }
                // in both cases, keep pA_start to p-1 and
                // p+1 to pA_end-1.  If the entry is not found, then
                // p == pA_end, and all entries are kept.

            #endif
        }

        //----------------------------------------------------------------------
        // log the result for the kth vector
        //----------------------------------------------------------------------

        Zp [k] = p ;
        Cp [k] = cjnz ;
    }

    //--------------------------------------------------------------------------
    // compute Wfirst and Wlast for each task
    //--------------------------------------------------------------------------

    // Wfirst [0..A_ntasks-1] and Wlast [0..A_ntasks-1] are required for
    // constructing C_start_slice [0..A_ntasks-1] in GB_selector.

    for (int tid = 0 ; tid < A_ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_Aslice [tid] ;
        int64_t klast  = klast_Aslice  [tid] ;
        Wfirst [tid] = 0 ;
        Wlast  [tid] = 0 ;

        if (kfirst <= klast)
        {
            int64_t pA_start = pstart_Aslice [tid] ;
            int64_t pA_end   = GBP (Ap, kfirst+1, avlen) ;
            pA_end = GB_IMIN (pA_end, pstart_Aslice [tid+1]) ;
            if (pA_start < pA_end)
            { 
                #if defined ( GB_TRIL_SELECTOR  ) || \
                    defined ( GB_ROWGT_SELECTOR )

                    // keep Zp [kfirst] to pA_end-1
                    int64_t p = GB_IMAX (Zp [kfirst], pA_start) ;
                    Wfirst [tid] = GB_IMAX (0, pA_end - p) ;

                #elif defined ( GB_TRIU_SELECTOR  ) || \
                      defined ( GB_ROWLE_SELECTOR )

                    // keep pA_start to Zp [kfirst]-1
                    int64_t p = GB_IMIN (Zp [kfirst], pA_end) ;
                    Wfirst [tid] = GB_IMAX (0, p - pA_start) ;

                #elif defined ( GB_DIAG_SELECTOR )

                    // task that owns the diagonal entry does this work
                    int64_t p = Zp [kfirst] ;
                    Wfirst [tid] = (pA_start <= p && p < pA_end) ? 1 : 0 ;

                #elif defined ( GB_OFFDIAG_SELECTOR  ) || \
                      defined ( GB_ROWINDEX_SELECTOR )

                    // keep pA_start to Zp [kfirst]-1
                    int64_t p = GB_IMIN (Zp [kfirst], pA_end) ;
                    Wfirst [tid] = GB_IMAX (0, p - pA_start) ;

                    // keep Zp [kfirst]+1 to pA_end-1
                    p = GB_IMAX (Zp [kfirst]+1, pA_start) ;
                    Wfirst [tid] += GB_IMAX (0, pA_end - p) ;

                #endif
            }
        }

        if (kfirst < klast)
        {
            int64_t pA_start = GBP (Ap, klast, avlen) ;
            int64_t pA_end   = pstart_Aslice [tid+1] ;
            if (pA_start < pA_end)
            { 
                #if defined ( GB_TRIL_SELECTOR  ) || \
                    defined ( GB_ROWGT_SELECTOR )

                    // keep Zp [klast] to pA_end-1
                    int64_t p = GB_IMAX (Zp [klast], pA_start) ;
                    Wlast [tid] = GB_IMAX (0, pA_end - p) ;

                #elif defined ( GB_TRIU_SELECTOR  ) || \
                      defined ( GB_ROWLE_SELECTOR )

                    // keep pA_start to Zp [klast]-1
                    int64_t p = GB_IMIN (Zp [klast], pA_end) ;
                    Wlast [tid] = GB_IMAX (0, p - pA_start) ;

                #elif defined ( GB_DIAG_SELECTOR )

                    // task that owns the diagonal entry does this work
                    int64_t p = Zp [klast] ;
                    Wlast [tid] = (pA_start <= p && p < pA_end) ? 1 : 0 ;

                #elif defined ( GB_OFFDIAG_SELECTOR  ) || \
                      defined ( GB_ROWINDEX_SELECTOR )

                    // keep pA_start to Zp [klast]-1
                    int64_t p = GB_IMIN (Zp [klast], pA_end) ;
                    Wlast [tid] = GB_IMAX (0, p - pA_start) ;

                    // keep Zp [klast]+1 to pA_end-1
                    p = GB_IMAX (Zp [klast]+1, pA_start) ;
                    Wlast [tid] += GB_IMAX (0, pA_end - p) ;

                #endif
            }
        }
    }

#endif


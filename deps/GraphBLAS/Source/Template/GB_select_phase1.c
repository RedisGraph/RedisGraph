//------------------------------------------------------------------------------
// GB_select_count: count entries in eacn vector for C=select(A,thunk)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#if defined ( GB_ENTRY_SELECTOR )

    #define GB_CTYPE int64_t
    #include "GB_reduce_each_vector.c"

#else

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    int64_t anvec = A->nvec ;
    int64_t avlen = A->vlen ;

    //--------------------------------------------------------------------------
    // tril, triu, diag, offdiag, resize: binary search in each vector
    //--------------------------------------------------------------------------

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(guided)
    for (k = 0 ; k < anvec ; k++)
    {

        //----------------------------------------------------------------------
        // get A(:,k)
        //----------------------------------------------------------------------

        int64_t pA_start = Ap [k] ;
        int64_t pA_end   = Ap [k+1] ;
        int64_t p = pA_start ;
        int64_t cjnz = 0 ;
        int64_t ajnz = pA_end - pA_start ;
        bool found = false ;

        if (ajnz > 0)
        {

            //------------------------------------------------------------------
            // search for the entry A(i,k)
            //------------------------------------------------------------------

            int64_t ifirst = Ai [pA_start] ;
            int64_t ilast  = Ai [pA_end-1] ;

            #if defined ( GB_RESIZE_SELECTOR )
            int64_t i = ithunk ;
            #else
            int64_t j = (Ah == NULL) ? k : Ah [k] ;
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
                ASSERT (Ai [p] == i) ;
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

            #elif defined ( GB_TRIU_SELECTOR ) \
               || defined ( GB_RESIZE_SELECTOR )

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

            #elif defined ( GB_OFFDIAG_SELECTOR )

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

    // Wfirst [0..ntasks-1] and Wlast [0..ntasks-1] are required for
    // constructing C_start_slice [0..ntasks-1] in GB_selector.

    int64_t *GB_RESTRICT Wfirst = (int64_t *) Wfirst_space ;
    int64_t *GB_RESTRICT Wlast  = (int64_t *) Wlast_space  ;

    for (int tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        if (kfirst <= klast)
        {
            int64_t pA_start = pstart_slice [tid] ;
            int64_t pA_end = GB_IMIN (Ap [kfirst+1], pstart_slice [tid+1]) ;
            if (pA_start < pA_end)
            { 
                #if defined ( GB_TRIL_SELECTOR )

                    // keep Zp [kfirst] to pA_end-1
                    int64_t p = GB_IMAX (Zp [kfirst], pA_start) ;
                    Wfirst [tid] = GB_IMAX (0, pA_end - p) ;

                #elif defined ( GB_TRIU_SELECTOR ) \
                   || defined ( GB_RESIZE_SELECTOR )

                    // keep pA_start to Zp [kfirst]-1
                    int64_t p = GB_IMIN (Zp [kfirst], pA_end) ;
                    Wfirst [tid] = GB_IMAX (0, p - pA_start) ;

                #elif defined ( GB_DIAG_SELECTOR )

                    // task that owns the diagonal entry does this work
                    int64_t p = Zp [kfirst] ;
                    Wfirst [tid] = (pA_start <= p && p < pA_end) ? 1 : 0 ;

                #elif defined ( GB_OFFDIAG_SELECTOR )

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
            int64_t pA_start = Ap [klast] ;
            int64_t pA_end   = pstart_slice [tid+1] ;
            if (pA_start < pA_end)
            { 
                #if defined ( GB_TRIL_SELECTOR )

                    // keep Zp [klast] to pA_end-1
                    int64_t p = GB_IMAX (Zp [klast], pA_start) ;
                    Wlast [tid] = GB_IMAX (0, pA_end - p) ;

                #elif defined ( GB_TRIU_SELECTOR ) \
                   || defined ( GB_RESIZE_SELECTOR )

                    // keep pA_start to Zp [klast]-1
                    int64_t p = GB_IMIN (Zp [klast], pA_end) ;
                    Wlast [tid] = GB_IMAX (0, p - pA_start) ;

                #elif defined ( GB_DIAG_SELECTOR )

                    // task that owns the diagonal entry does this work
                    int64_t p = Zp [klast] ;
                    Wlast [tid] = (pA_start <= p && p < pA_end) ? 1 : 0 ;

                #elif defined ( GB_OFFDIAG_SELECTOR )

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


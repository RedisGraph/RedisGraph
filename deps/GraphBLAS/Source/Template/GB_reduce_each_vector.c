//------------------------------------------------------------------------------
// GB_reduce_each_vector: Tx(j)=reduce(A(:,j)), reduce a matrix to a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce a matrix to a vector.  The kth vector A(:,k) is reduced to the kth
// scalar Tx(k).  Each thread computes the reductions on roughly the same number
// of entries, which means that a vector A(:,k) may be reduced by more than one
// thread.  The first vector A(:,kfirst) reduced by thread tid may be partial,
// where the prior thread tid-1 (and other prior threads) may also do some of
// the reductions for this same vector A(:,kfirst).  The thread tid fully
// reduces all vectors A(:,k) for k in the range kfirst+1 to klast-1.  The last
// vector A(:,klast) reduced by thread tid may also be partial.  Thread tid+1,
// and following threads, may also do some of the reduces for A(:,klast).

#ifndef GB_GET_J
#define GB_GET_J ;
#endif

{

    // Ah, Ai, asize, avlen, avdim unused for some uses of this template
    #include "GB_unused.h"

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = A->x ;
    size_t  asize = A->type->size ;
    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;

    //--------------------------------------------------------------------------
    // workspace for first and last vectors of each slice
    //--------------------------------------------------------------------------

    // ztype Wfirst [ntasks], Wlast [ntasks] ;
    GB_CTYPE *GB_RESTRICT Wfirst = (GB_CTYPE *) Wfirst_space ;
    GB_CTYPE *GB_RESTRICT Wlast  = (GB_CTYPE *) Wlast_space ;

    //--------------------------------------------------------------------------
    // reduce each slice
    //--------------------------------------------------------------------------

    // each thread reduces its own part in parallel
    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then thread tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        //----------------------------------------------------------------------
        // reduce vectors kfirst to klast
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) to be reduced by this thread
            //------------------------------------------------------------------

            GB_GET_J ;
            int64_t pA_start, pA_end ;
            GB_get_pA_and_pC (&pA_start, &pA_end, NULL,
                tid, k, kfirst, klast, pstart_slice, NULL, NULL, Ap) ;

            //------------------------------------------------------------------
            // reduce Ax [pA_start ... pA_end-1] to a scalar, if non-empty
            //------------------------------------------------------------------

            if (pA_start < pA_end)
            {

                //--------------------------------------------------------------
                // reduce the vector to the scalar s
                //--------------------------------------------------------------

                // ztype s = (ztype) Ax [pA_start], with typecast
                GB_SCALAR (s) ;
                GB_CAST_ARRAY_TO_SCALAR (s, Ax, pA_start) ;
                for (int64_t p = pA_start+1 ; p < pA_end ; p++)
                { 
                    // check for early exit
                    GB_BREAK_IF_TERMINAL (s) ;
                    // s += (ztype) Ax [p], with typecast
                    GB_ADD_CAST_ARRAY_TO_SCALAR (s, Ax, p) ;
                }

                //--------------------------------------------------------------
                // save the result s
                //--------------------------------------------------------------

                if (k == kfirst)
                { 
                    // Wfirst [tid] = s ; no typecast
                    GB_COPY_SCALAR_TO_ARRAY (Wfirst, tid, s) ;
                }
                else if (k == klast)
                { 
                    // Wlast [tid] = s ; no typecast
                    GB_COPY_SCALAR_TO_ARRAY (Wlast, tid, s) ;
                }
                else
                { 
                    // Tx [k] = s ; no typecast
                    GB_COPY_SCALAR_TO_ARRAY (Tx, k, s) ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // reduce the first and last vector of each slice using a single thread
    //--------------------------------------------------------------------------

    // This step is sequential, but it takes only O(ntasks) time.  The only
    // case where this could be a problem is if a user-defined operator was
    // a very costly one.

    int64_t kprior = -1 ;

    for (int tid = 0 ; tid < ntasks ; tid++)
    {

        //----------------------------------------------------------------------
        // sum up the partial result that thread tid computed for kfirst
        //----------------------------------------------------------------------

        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        if (kfirst <= klast)
        {
            int64_t pA_start = pstart_slice [tid] ;
            int64_t pA_end   = GB_IMIN (Ap [kfirst+1], pstart_slice [tid+1]) ;
            if (pA_start < pA_end)
            {
                if (kprior < kfirst)
                { 
                    // This thread is the first one that did work on
                    // A(:,kfirst), so use it to start the reduction.
                    // Tx [kfirst] = Wfirst [tid], no typecast
                    GB_COPY_ARRAY_TO_ARRAY (Tx, kfirst, Wfirst, tid) ;
                }
                else
                { 
                    // Tx [kfirst] += Wfirst [tid], no typecast
                    GB_ADD_ARRAY_TO_ARRAY (Tx, kfirst, Wfirst, tid) ;
                }
                kprior = kfirst ;
            }
        }

        //----------------------------------------------------------------------
        // sum up the partial result that thread tid computed for klast
        //----------------------------------------------------------------------

        if (kfirst < klast)
        {
            int64_t pA_start = Ap [klast] ;
            int64_t pA_end   = pstart_slice [tid+1] ;
            if (pA_start < pA_end)
            {
                /* if */ ASSERT (kprior < klast) ;
                { 
                    // This thread is the first one that did work on
                    // A(:,klast), so use it to start the reduction.
                    // Tx [klast] = Wlast [tid], no typecast
                    GB_COPY_ARRAY_TO_ARRAY (Tx, klast, Wlast, tid) ;
                }
                /*
                else
                {
                    // If kfirst < klast and A(:,klast is not empty, then this
                    // task is always the first one to do work on A(:,klast),
                    // so this case is never used.
                    ASSERT (GB_DEAD_CODE) ;
                    // Tx [klast] += Wlast [tid], no typecast
                    GB_ADD_ARRAY_TO_ARRAY (Tx, klast, Wlast, tid) ;
                }
                */
                kprior = klast ;
            }
        }
    }
}


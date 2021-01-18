//------------------------------------------------------------------------------
// GB_bitmap_assign_A_whole_template: traverse A for bitmap assignment into C
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This template traverses over all the entries of the matrix A and operates on
// the corresponding entry in C(i,j), using the GB_AIJ_WORK macro.  A can be
// hypersparse or sparse, not bitmap or full.  It is not a scalar.

{

    //--------------------------------------------------------------------------
    // matrix assignment: slice the entries of A for each task
    //--------------------------------------------------------------------------

    const int64_t avlen = A->vlen ;
    int nthreads = GB_nthreads (GB_NNZ (A) + A->nvec, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (8 * nthreads) ;
    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, A, &ntasks))
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // traverse of the entries of the matrix A
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:cnvals)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;
        int64_t task_cnvals = 0 ;

        //----------------------------------------------------------------------
        // traverse over A (:,kfirst:klast)
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) for this task
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA_start, pA_end ;
            GB_get_pA (&pA_start, &pA_end, tid, k, kfirst,
                klast, pstart_slice, Ap, avlen) ;

            //------------------------------------------------------------------
            // traverse over A(:,j), the kth vector of A
            //------------------------------------------------------------------

            int64_t pC0 = j * cvlen ;      // first entry in C(:,j)

            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
            { 
                int64_t i = Ai [pA] ;
                int64_t pC = i + pC0 ;
                // operate on C(i,j) at pC, and A(i,j) at pA.  The mask
                // can be accessed at pC if M is bitmap or full.  A has any
                // sparsity format so only A(i,j) can be accessed at pA.
                GB_AIJ_WORK (pC, pA) ;
            }
        }
        cnvals += task_cnvals ;
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;
}


//------------------------------------------------------------------------------
// GB_positional_op_ijp: C = positional_op (A), depending j
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// TODO: rename, and use #ifdef instead of offset = 0 or 1.
// TODO: use this kernel for GrB_extractTuples, to create J array.

// A can be jumbled.  If A is jumbled, so is C.
// if A and C are bitmap, not all of Cx need to be written to, but it's faster
// just to write to all of it.  C->b is copied from A->b in the caller.

{

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, A, &ntasks))
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // Cx = positional_op (A)
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        //----------------------------------------------------------------------
        // C(:,kfirst:klast) = op (A(:,kfirst:klast))
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) and Cx to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA_start, pA_end ;
            GB_get_pA (&pA_start, &pA_end, tid, k,
                kfirst, klast, pstart_slice, Ap, avlen) ;

            //------------------------------------------------------------------
            // C(:,j) = op (A(:,j))
            //------------------------------------------------------------------

            GB_PRAGMA_SIMD
            for (int64_t p = pA_start ; p < pA_end ; p++)
            { 
                // GB_POSITION is j or j+1
                Cx_int [p] = GB_POSITION ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;
}

#undef GB_POSITION


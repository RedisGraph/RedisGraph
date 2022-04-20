//------------------------------------------------------------------------------
// GB_convert_sparse_to_bitmap_template: convert A from sparse to bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    #if defined ( GB_ATYPE )
    const GB_ATYPE *restrict Axold = (GB_ATYPE *) A->x ;
          GB_ATYPE *restrict Axnew = (GB_ATYPE *) Ax_new ;
    #endif

    int tid ;
    #pragma omp parallel for num_threads(A_nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < A_ntasks ; tid++)
    {
        int64_t kfirst = kfirst_Aslice [tid] ;
        int64_t klast  = klast_Aslice  [tid] ;
        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,j) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA_start, pA_end ;
            GB_get_pA (&pA_start, &pA_end, tid, k,
                kfirst, klast, pstart_Aslice, Ap, avlen) ;

            // the start of A(:,j) in the new bitmap
            int64_t pA_new = j * avlen ;

            //------------------------------------------------------------------
            // convert A(:,j) from sparse to bitmap
            //------------------------------------------------------------------

            if (nzombies == 0)
            {
                for (int64_t p = pA_start ; p < pA_end ; p++)
                { 
                    // A(i,j) has index i, value Axold [p]
                    int64_t i = Ai [p] ;
                    int64_t pnew = i + pA_new ;
                    // move A(i,j) to its new place in the bitmap
                    // Axnew [pnew] = Axold [p]
                    GB_COPY (Axnew, pnew, Axold, p) ;
                    Ab [pnew] = 1 ;
                }
            }
            else
            {
                for (int64_t p = pA_start ; p < pA_end ; p++)
                {
                    // A(i,j) has index i, value Axold [p]
                    int64_t i = Ai [p] ;
                    if (!GB_IS_ZOMBIE (i))
                    { 
                        int64_t pnew = i + pA_new ;
                        // move A(i,j) to its new place in the bitmap
                        // Axnew [pnew] = Axold [p]
                        GB_COPY (Axnew, pnew, Axold, p) ;
                        Ab [pnew] = 1 ;
                    }
                }
            }
        }
    }

    done = true ;
}

#undef GB_ATYPE


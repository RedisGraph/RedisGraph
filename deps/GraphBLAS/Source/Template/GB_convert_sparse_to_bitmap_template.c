//------------------------------------------------------------------------------
// GB_convert_sparse_to_bitmap_template: convert A from sparse to bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) A->x ;
    const int64_t avlen = A->vlen ;
    const int64_t nzombies = A->nzombies ;

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;
        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,j) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA_start, pA_end ;
            GB_get_pA (&pA_start, &pA_end, tid, k,
                kfirst, klast, pstart_slice, Ap, avlen) ;

            // the start of A(:,j) in the new bitmap
            int64_t pA_new = j * avlen ;

            //------------------------------------------------------------------
            // convert A(:,j) from sparse to bitmap
            //------------------------------------------------------------------

            if (nzombies == 0)
            {
                for (int64_t p = pA_start ; p < pA_end ; p++)
                { 
                    // A(i,j) has index i, value Ax [p]
                    int64_t i = Ai [p] ;
                    int64_t pnew = i + pA_new ;
                    // move A(i,j) to its new place in the bitmap
                    // Ax_new [pnew] = Ax [p]
                    GB_COPY_A_TO_C (Ax_new, pnew, Ax, p) ;
                    Ab [pnew] = 1 ;
                }
            }
            else
            {
                for (int64_t p = pA_start ; p < pA_end ; p++)
                { 
                    // A(i,j) has index i, value Ax [p]
                    int64_t i = Ai [p] ;
                    if (!GB_IS_ZOMBIE (i))
                    { 
                        int64_t pnew = i + pA_new ;
                        // move A(i,j) to its new place in the bitmap
                        // Ax_new [pnew] = Ax [p]
                        GB_COPY_A_TO_C (Ax_new, pnew, Ax, p) ;
                        Ab [pnew] = 1 ;
                    }
                }
            }
        }
    }
}


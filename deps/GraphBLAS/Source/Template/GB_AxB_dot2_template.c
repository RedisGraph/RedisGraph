//------------------------------------------------------------------------------
// GB_AxB_dot2_template:  C=A'B, C<!M>=A'*B, or C<M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A and B are sparse, bitmap, or full; never hypersparse.  If the input
// matrices A and/or B are hypersparse, they are packed into sparse matrices,
// and C is unpacked from bitmap to sparse/hypersparse when done.

#if ( !GB_A_IS_HYPER && !GB_B_IS_HYPER )
{

    //--------------------------------------------------------------------------
    // C=A'*B, C<M>=A'*B, or C<!M>=A'*B where C is bitmap
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:cnvals)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        const int a_tid = tid / nbslice ;
        const int b_tid = tid % nbslice ;
        const int64_t kA_start = A_slice [a_tid] ;
        const int64_t kA_end   = A_slice [a_tid+1] ;
        const int64_t kB_start = B_slice [b_tid] ;
        const int64_t kB_end   = B_slice [b_tid+1] ;
        int64_t task_cnvals = 0 ;

        //----------------------------------------------------------------------
        // C=A'*B, C<M>=A'*B, or C<!M>=A'*B via dot products
        //----------------------------------------------------------------------

        for (int64_t j = kB_start ; j < kB_end ; j++)
        {

            //------------------------------------------------------------------
            // get B(:,j) and C(:,j)
            //------------------------------------------------------------------

            const int64_t pC_start = j * cvlen ;

            #if GB_B_IS_SPARSE
                // B is sparse (never hypersparse)
                const int64_t pB_start = Bp [j] ;
                const int64_t pB_end = Bp [j+1] ;
                const int64_t bjnz = pB_end - pB_start ;
                if (bjnz == 0)
                { 
                    // no work to do if B(:,j) is empty, except to clear Cb
                    memset (&Cb [pC_start + kA_start], 0, kA_end - kA_start) ;
                    continue ;
                }
                #if GB_A_IS_SPARSE
                    // Both A and B are sparse; get first and last in B(:,j)
                    const int64_t ib_first = Bi [pB_start] ;
                    const int64_t ib_last  = Bi [pB_end-1] ;
                #endif
            #else
                // B is bitmap or full
                const int64_t pB_start = j * vlen ;
            #endif

            //------------------------------------------------------------------
            // C(:,j)<#M(:,j)> = A'*B(:,j), or C(:,j) = A'*B(:,j) if no mask
            //------------------------------------------------------------------

            for (int64_t i = kA_start ; i < kA_end ; i++)
            {

                //--------------------------------------------------------------
                // get C(i,j), M(i,j), and clear the C(i,j) bitmap
                //--------------------------------------------------------------

                int64_t pC = pC_start + i ;     // C is bitmap

                #if defined ( GB_ANY_SPECIALIZED )
                // M is bitmap and structural; Mask_comp true
                Cb [pC] = 0 ;
                if (!Mb [pC])
                #elif defined ( GB_MASK_IS_PRESENT )
                bool mij ;
                if (M_is_bitmap)
                { 
                    // M is bitmap
                    mij = Mb [pC] && GB_mcast (Mx, pC, msize) ;
                }
                else if (M_is_full)
                { 
                    // M is full
                    mij = GB_mcast (Mx, pC, msize) ;
                }
                else // M is sparse or hyper
                { 
                    // M has been scattered into the C bitmap
                    mij = (Cb [pC] > 1) ;
                }
                Cb [pC] = 0 ;
                if (mij ^ Mask_comp)
                #else
                // M is not present
                Cb [pC] = 0 ;
                #endif
                { 

                    //----------------------------------------------------------
                    // the mask allows C(i,j) to be computed
                    //----------------------------------------------------------

                    #if GB_A_IS_SPARSE
                    // A is sparse
                    int64_t pA = Ap [i] ;
                    const int64_t pA_end = Ap [i+1] ;
                    const int64_t ainz = pA_end - pA ;
                    if (ainz > 0)
                    #else
                    // A is bitmap or full
                    const int64_t pA = i * vlen ;
                    #endif
                    { 
                        // C(i,j) = A(:,i)'*B(:,j)
                        bool cij_exists = false ;
                        GB_CIJ_DECLARE (cij) ;
                        #include "GB_AxB_dot_cij.c"
                    }
                }
            }
        }
        cnvals += task_cnvals ;
    }
}
#endif

#undef GB_A_IS_SPARSE
#undef GB_A_IS_HYPER
#undef GB_A_IS_BITMAP
#undef GB_A_IS_FULL
#undef GB_B_IS_SPARSE
#undef GB_B_IS_HYPER
#undef GB_B_IS_BITMAP
#undef GB_B_IS_FULL


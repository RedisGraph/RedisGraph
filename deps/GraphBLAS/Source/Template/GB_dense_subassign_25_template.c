//------------------------------------------------------------------------------
// GB_dense_subassign_25_template: C<M> = A where C is empty and A is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C<M> = A where C starts as empty, M is structural, and A is dense.  The
// pattern of C is an exact copy of M.  A is full, dense, or bitmap.
// M is sparse or hypersparse, and C is constructed with the same pattern as M.

{

    //--------------------------------------------------------------------------
    // get C, M, and A
    //--------------------------------------------------------------------------

    ASSERT (GB_sparsity (M) == GB_sparsity (C)) ;
    int64_t *restrict Ci = C->i ;

    ASSERT (GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mh = M->h ;
    const int64_t *restrict Mi = M->i ;
    const int64_t mvlen = M->vlen ;

    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_iso = A->iso ;
    const int8_t   *restrict Ab = A->b ;
    const int64_t avlen = A->vlen ;

    const int64_t *restrict kfirst_Mslice = M_ek_slicing ;
    const int64_t *restrict klast_Mslice  = M_ek_slicing + M_ntasks ;
    const int64_t *restrict pstart_Mslice = M_ek_slicing + M_ntasks * 2 ;

    #ifdef GB_ISO_ASSIGN
    ASSERT (C->iso) ;
    #else
    ASSERT (!C->iso) ;
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    //--------------------------------------------------------------------------
    // C<M> = A
    //--------------------------------------------------------------------------

    if (A_is_bitmap)
    {

        //----------------------------------------------------------------------
        // A is bitmap, so zombies can be created in C
        //----------------------------------------------------------------------

        int64_t nzombies = 0 ;

        int tid ;
        #pragma omp parallel for num_threads(M_nthreads) schedule(dynamic,1) \
            reduction(+:nzombies)
        for (tid = 0 ; tid < M_ntasks ; tid++)
        {

            // if kfirst > klast then task tid does no work at all
            int64_t kfirst = kfirst_Mslice [tid] ;
            int64_t klast  = klast_Mslice  [tid] ;
            int64_t task_nzombies = 0 ;

            //------------------------------------------------------------------
            // C<M(:,kfirst:klast)> = A(:,kfirst:klast)
            //------------------------------------------------------------------

            for (int64_t k = kfirst ; k <= klast ; k++)
            {

                //--------------------------------------------------------------
                // find the part of M(:,k) to be operated on by this task
                //--------------------------------------------------------------

                int64_t j = GBH (Mh, k) ;
                int64_t pM_start, pM_end ;
                GB_get_pA (&pM_start, &pM_end, tid, k,
                    kfirst, klast, pstart_Mslice, Mp, mvlen) ;

                //--------------------------------------------------------------
                // C<M(:,j)> = A(:,j)
                //--------------------------------------------------------------

                // M is hypersparse or sparse.  C is the same as M.
                // pA points to the start of A(:,j) since A is dense
                int64_t pA = j * avlen ;
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    int64_t i = Mi [pM] ;
                    int64_t p = pA + i ;
                    if (Ab [p])
                    { 
                        // C(i,j) = A(i,j)
                        #ifndef GB_ISO_ASSIGN
                        GB_COPY_A_TO_C (Cx, pM, Ax, p, A_iso) ;
                        #endif
                    }
                    else
                    { 
                        // C(i,j) becomes a zombie
                        task_nzombies++ ;
                        Ci [pM] = GB_FLIP (i) ;
                    }
                }
            }
            nzombies += task_nzombies ;
        }
        C->nzombies = nzombies ;

    }
    else
    {

        //----------------------------------------------------------------------
        // A is full, so no zombies will appear in C
        //----------------------------------------------------------------------

        #ifndef GB_ISO_ASSIGN
        {

            int tid ;
            #pragma omp parallel for num_threads(M_nthreads) schedule(dynamic,1)
            for (tid = 0 ; tid < M_ntasks ; tid++)
            {

                // if kfirst > klast then task tid does no work at all
                int64_t kfirst = kfirst_Mslice [tid] ;
                int64_t klast  = klast_Mslice  [tid] ;

                //--------------------------------------------------------------
                // C<M(:,kfirst:klast)> = A(:,kfirst:klast)
                //--------------------------------------------------------------

                for (int64_t k = kfirst ; k <= klast ; k++)
                {

                    //----------------------------------------------------------
                    // find the part of M(:,k) to be operated on by this task
                    //----------------------------------------------------------

                    int64_t j = GBH (Mh, k) ;
                    int64_t pM_start, pM_end ;
                    GB_get_pA (&pM_start, &pM_end, tid, k,
                        kfirst, klast, pstart_Mslice, Mp, mvlen) ;

                    //----------------------------------------------------------
                    // C<M(:,j)> = A(:,j)
                    //----------------------------------------------------------

                    // M is hypersparse or sparse.  C is the same as M.
                    // pA points to the start of A(:,j) since A is dense
                    int64_t pA = j * avlen ;
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                    { 
                        // C(i,j) = A(i,j)
                        int64_t p = pA + GBI (Mi, pM, mvlen) ;
                        GB_COPY_A_TO_C (Cx, pM, Ax, p, A_iso) ;
                    }
                }
            }
        }
        #endif
    }
}

#undef GB_ISO_ASSIGN


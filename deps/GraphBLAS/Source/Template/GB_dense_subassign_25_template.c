//------------------------------------------------------------------------------
// GB_dense_subassign_25_template: C<M> = A where C is empty and A is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
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
    GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;
    int64_t *GB_RESTRICT Ci = C->i ;

    ASSERT (GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    const int64_t *GB_RESTRICT Mp = M->p ;
    const int64_t *GB_RESTRICT Mh = M->h ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const int64_t mvlen = M->vlen ;

    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) A->x ;
    const int8_t   *GB_RESTRICT Ab = A->b ;
    const int64_t avlen = A->vlen ;

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
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:nzombies)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            // if kfirst > klast then task tid does no work at all
            int64_t kfirst = kfirst_slice [tid] ;
            int64_t klast  = klast_slice  [tid] ;
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
                    kfirst, klast, pstart_slice, Mp, mvlen) ;

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
                        GB_COPY_A_TO_C (Cx, pM, Ax, p) ;    // Cx [pM] = Ax [p]
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

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            // if kfirst > klast then task tid does no work at all
            int64_t kfirst = kfirst_slice [tid] ;
            int64_t klast  = klast_slice  [tid] ;

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
                    kfirst, klast, pstart_slice, Mp, mvlen) ;

                //--------------------------------------------------------------
                // C<M(:,j)> = A(:,j)
                //--------------------------------------------------------------

                // M is hypersparse or sparse.  C is the same as M.
                // pA points to the start of A(:,j) since A is dense
                int64_t pA = j * avlen ;
                GB_PRAGMA_SIMD_VECTORIZE
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                { 
                    int64_t p = pA + GBI (Mi, pM, mvlen) ;
                    GB_COPY_A_TO_C (Cx, pM, Ax, p) ;    // Cx [pM] = Ax [p]
                }
            }
        }
    }
}


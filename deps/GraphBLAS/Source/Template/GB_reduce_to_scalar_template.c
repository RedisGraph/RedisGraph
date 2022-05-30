//------------------------------------------------------------------------------
// GB_reduce_to_scalar_template: s=reduce(A), reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Reduce a matrix to a scalar, with typecasting and generic operators.
// No panel is used.

{

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const int8_t   *restrict Ab = A->b ;
    const int64_t  *restrict Ai = A->i ;
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    int64_t anz = GB_nnz_held (A) ;
    ASSERT (anz > 0) ;
    const bool A_has_zombies = (A->nzombies > 0) ;
    ASSERT (!A->iso) ;

    //--------------------------------------------------------------------------
    // reduce A to a scalar
    //--------------------------------------------------------------------------

    if (nthreads == 1)
    {

        //----------------------------------------------------------------------
        // single thread
        //----------------------------------------------------------------------

        for (int64_t p = 0 ; p < anz ; p++)
        { 
            // skip if the entry is a zombie or if not in the bitmap
            if (A_has_zombies && GB_IS_ZOMBIE (Ai [p])) continue ;
            if (!GBB (Ab, p)) continue ;
            // s = op (s, (ztype) Ax [p])
            GB_ADD_CAST_ARRAY_TO_SCALAR (s, Ax, p) ;
            // check for early exit
            #if GB_HAS_TERMINAL
            if (GB_IS_TERMINAL (s)) break ;
            #endif
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // each thread reduces its own slice in parallel
        //----------------------------------------------------------------------

        bool early_exit = false ;
        int tid ;

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (tid = 0 ; tid < ntasks ; tid++)
        {
            int64_t pstart, pend ;
            GB_PARTITION (pstart, pend, anz, tid, ntasks) ;
            // ztype t = identity
            GB_SCALAR_IDENTITY (t) ;
            bool my_exit, found = false ;
            GB_ATOMIC_READ
            my_exit = early_exit ;
            if (!my_exit)
            {
                for (int64_t p = pstart ; p < pend ; p++)
                { 
                    // skip if the entry is a zombie or if not in the bitmap
                    if (A_has_zombies && GB_IS_ZOMBIE (Ai [p])) continue ;
                    if (!GBB (Ab, p)) continue ;
                    found = true ;
                    // t = op (t, (ztype) Ax [p]), with typecast
                    GB_ADD_CAST_ARRAY_TO_SCALAR (t, Ax, p) ;
                    // check for early exit
                    #if GB_HAS_TERMINAL
                    if (GB_IS_TERMINAL (t))
                    { 
                        // tell the other tasks to exit early
                        GB_ATOMIC_WRITE
                        early_exit = true ;
                        break ;
                    }
                    #endif
                }
            }
            F [tid] = found ;
            // W [tid] = t, no typecast
            GB_COPY_SCALAR_TO_ARRAY (W, tid, t) ;
        }

        //----------------------------------------------------------------------
        // sum up the results of each slice using a single thread
        //----------------------------------------------------------------------

        for (int tid = 0 ; tid < ntasks ; tid++)
        {
            if (F [tid])
            { 
                // s = op (s, W [tid]), no typecast
                GB_ADD_ARRAY_TO_SCALAR (s, W, tid) ;
            }
        }
    }
}


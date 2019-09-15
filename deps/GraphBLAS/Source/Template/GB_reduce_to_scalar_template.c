//------------------------------------------------------------------------------
// GB_reduce_to_scalar_template: s=reduce(A), reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce a matrix to a scalar, with typecasting and generic operators.
// No panel is used.

{

    const GB_ATYPE *restrict Ax = A->x ;
    int64_t anz = GB_NNZ (A) ;
    ASSERT (anz > 0) ;

    if (nthreads == 1)
    {

        //----------------------------------------------------------------------
        // single thread
        //----------------------------------------------------------------------

        // s = (ztype) Ax [0]
        GB_CAST_ARRAY_TO_SCALAR (s, Ax, 0) ;
        for (int64_t p = 1 ; p < anz ; p++)
        { 
            // check for early exit
            GB_BREAK_IF_TERMINAL (s) ;
            // s = op (s, (ztype) Ax [p])
            GB_ADD_CAST_ARRAY_TO_SCALAR (s, Ax, p) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // create workspace for multiple threads
        //----------------------------------------------------------------------

        // ztype W [ntasks] ;
        GB_REDUCTION_WORKSPACE (W, ntasks) ;
        ASSERT (ntasks <= anz) ;
        bool early_exit = false ;

        //----------------------------------------------------------------------
        // each thread reduces its own slice in parallel
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (int tid = 0 ; tid < ntasks ; tid++)
        {
            int64_t pstart, pend ;
            GB_PARTITION (pstart, pend, anz, tid, ntasks) ;
            // ztype t = (ztype) Ax [pstart], with typecast
            GB_SCALAR (t) ;
            GB_CAST_ARRAY_TO_SCALAR (t, Ax, pstart) ;
            GB_IF_NOT_EARLY_EXIT
            {
                for (int64_t p = pstart+1 ; p < pend ; p++)
                { 
                    // check for early exit
                    GB_PARALLEL_BREAK_IF_TERMINAL (t) ;
                    // t = op (t, (ztype) Ax [p]), with typecast
                    GB_ADD_CAST_ARRAY_TO_SCALAR (t, Ax, p) ;
                }
            }
            // W [tid] = t, no typecast
            GB_COPY_SCALAR_TO_ARRAY (W, tid, t) ;
        }

        //----------------------------------------------------------------------
        // sum up the results of each slice using a single thread
        //----------------------------------------------------------------------

        // s = W [0], no typecast
        GB_COPY_ARRAY_TO_SCALAR (s, W, 0) ;
        for (int tid = 1 ; tid < ntasks ; tid++)
        { 
            // s = op (s, W [tid]), no typecast
            GB_ADD_ARRAY_TO_SCALAR (s, W, tid) ;
        }
    }
}


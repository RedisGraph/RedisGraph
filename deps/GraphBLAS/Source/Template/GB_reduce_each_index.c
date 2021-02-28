//------------------------------------------------------------------------------
// GB_reduce_each_index: T(i)=reduce(A(i,:)), reduce a matrix to a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce a matrix to a vector.  All entries in A(i,:) are reduced to T(i).
// First, all threads reduce their slice to their own workspace, operating on
// roughly the same number of entries each.  The vectors in A are ignored; the
// reduction only depends on the indices.  Next, the threads cooperate to
// reduce all workspaces to the workspace of thread 0.  Finally, this last
// workspace is collected into T.

// If an out-of-memory condition occurs, the macro GB_FREE_ALL frees any
// workspace.  This has no effect on the built-in workers (GB_FREE_ALL does
// nothing), and the workspace is freed in the caller.  For the generic worker,
// the GB_FREE_ALL macro defined in GB_reduce_to_vector frees all workspace.

{

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const GB_ATYPE *GB_RESTRICT Ax = A->x ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const int64_t n = A->vlen ;
    size_t zsize = ttype->size ;

    //--------------------------------------------------------------------------
    // allocate workspace for each thread
    //--------------------------------------------------------------------------

    int ntasks = 256 * nthreads ;
    ntasks = GB_IMIN (ntasks, n) ;

    GB_CTYPE *GB_RESTRICT *Works = NULL ;      // size nth
    bool     *GB_RESTRICT *Marks = NULL ;      // size nth
    int64_t  *GB_RESTRICT Tnz = NULL ;         // size nth
    int64_t  *GB_RESTRICT Count = NULL ;       // size ntasks+1

    GB_CALLOC_MEMORY (Works, nth, sizeof (GB_CTYPE *)) ;
    GB_CALLOC_MEMORY (Marks, nth, sizeof (bool *)) ;
    GB_CALLOC_MEMORY (Tnz, nth, sizeof (int64_t)) ;
    GB_CALLOC_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
    bool ok = (Works != NULL && Marks != NULL && Tnz != NULL && Count != NULL) ;

    // This does not need to be parallel.  The calloc does not take O(n) time.
    if (ok)
    {
        for (int tid = 0 ; tid < nth ; tid++)
        { 
            GB_MALLOC_MEMORY (Works [tid], n, zsize) ;
            GB_CALLOC_MEMORY (Marks [tid], n, sizeof (bool)) ;
            ok = ok && (Works [tid] != NULL && Marks [tid] != NULL) ;
        }
    }

    if (!ok)
    {
        // out of memory
        if (Works != NULL)
        {
            for (int tid = 0 ; tid < nth ; tid++)
            { 
                GB_FREE_MEMORY (Works [tid], n, zsize) ;
            }
        }
        if (Marks != NULL)
        {
            for (int tid = 0 ; tid < nth ; tid++)
            { 
                GB_FREE_MEMORY (Marks [tid], n, sizeof (bool)) ;
            }
        }
        GB_FREE_MEMORY (Works, nth, sizeof (GB_CTYPE *)) ;
        GB_FREE_MEMORY (Marks, nth, sizeof (bool *)) ;
        GB_FREE_MEMORY (Tnz, nth, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // reduce each slice in its own workspace
    //--------------------------------------------------------------------------

    // each thread reduces its own slice in parallel
    int tid ;
    #pragma omp parallel for num_threads(nth) schedule(static)
    for (tid = 0 ; tid < nth ; tid++)
    {

        //----------------------------------------------------------------------
        // get the workspace for this thread
        //----------------------------------------------------------------------

        GB_CTYPE *GB_RESTRICT Work = Works [tid] ;
        bool     *GB_RESTRICT Mark = Marks [tid] ;
        int64_t my_tnz = 0 ;

        //----------------------------------------------------------------------
        // reduce the entries
        //----------------------------------------------------------------------

        for (int64_t p = pstart_slice [tid] ; p < pstart_slice [tid+1] ;p++)
        {
            int64_t i = Ai [p] ;
            // ztype aij = (ztype) Ax [p], with typecast
            GB_SCALAR (aij) ;
            GB_CAST_ARRAY_TO_SCALAR (aij, Ax, p) ;
            if (!Mark [i])
            { 
                // first time index i has been seen
                // Work [i] = aij ; no typecast
                GB_COPY_SCALAR_TO_ARRAY (Work, i, aij) ;
                Mark [i] = true ;
                my_tnz++ ;
            }
            else
            { 
                // Work [i] += aij ; no typecast
                GB_ADD_SCALAR_TO_ARRAY (Work, i, aij) ;
            }
        }
        Tnz [tid] = my_tnz ;
    }

    //--------------------------------------------------------------------------
    // reduce all workspace to Work [0] and count # entries in T
    //--------------------------------------------------------------------------

    GB_CTYPE *GB_RESTRICT Work0 = Works [0] ;
    bool     *GB_RESTRICT Mark0 = Marks [0] ;
    int64_t tnz = Tnz [0] ;

    if (nth > 1)
    {
        int64_t i ;
        #pragma omp parallel for num_threads(nthreads) schedule(static) \
            reduction(+:tnz)
        for (i = 0 ; i < n ; i++)
        {
            for (int tid = 1 ; tid < nth ; tid++)
            {
                const bool *GB_RESTRICT Mark = Marks [tid] ;
                if (Mark [i])
                {
                    // thread tid has a contribution to index i
                    const GB_CTYPE *GB_RESTRICT Work = Works [tid] ;
                    if (!Mark0 [i])
                    { 
                        // first time index i has been seen
                        // Work0 [i] = Work [i] ; no typecast
                        GB_COPY_ARRAY_TO_ARRAY (Work0, i, Work, i) ;
                        Mark0 [i] = true ;
                        tnz++ ;
                    }
                    else
                    { 
                        // Work0 [i] += Work [i] ; no typecast
                        GB_ADD_ARRAY_TO_ARRAY (Work0, i, Work, i) ;
                    }
                }
            }
        }

        // free all but workspace for thread 0
        for (int tid = 1 ; tid < nth ; tid++)
        {
            GB_FREE_MEMORY (Works [tid], n, zsize) ;
            GB_FREE_MEMORY (Marks [tid], n, sizeof (bool)) ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_FREE_MEMORY (Works, nth, sizeof (GB_CTYPE *)) ;
    GB_FREE_MEMORY (Marks, nth, sizeof (bool *)) ;
    GB_FREE_MEMORY (Tnz, nth, sizeof (int64_t)) ;

    //--------------------------------------------------------------------------
    // allocate T
    //--------------------------------------------------------------------------

    // since T is a GrB_Vector, it is CSC and not hypersparse
    GB_CREATE (&T, ttype, n, 1, GB_Ap_calloc, true,
        GB_FORCE_NONHYPER, GB_HYPER_DEFAULT, 1, tnz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_MEMORY (Work0, n, zsize) ;
        GB_FREE_MEMORY (Mark0, n, sizeof (bool)) ;
        GB_FREE_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    T->p [0] = 0 ;
    T->p [1] = tnz ;
    int64_t  *GB_RESTRICT Ti = T->i ;
    GB_CTYPE *GB_RESTRICT Tx = T->x ;
    T->nvec_nonempty = (tnz > 0) ? 1 : 0 ;

    //--------------------------------------------------------------------------
    // gather the results into T
    //--------------------------------------------------------------------------

    if (tnz == n)
    {

        //----------------------------------------------------------------------
        // T is dense: transplant Work0 into T->x
        //----------------------------------------------------------------------

        int64_t i ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (i = 0 ; i < n ; i++)
        { 
            Ti [i] = i ;
        }
        GB_FREE_MEMORY (T->x, n, zsize) ;
        T->x = Work0 ;
        Work0 = NULL ;

    }
    else
    {

        //----------------------------------------------------------------------
        // T is sparse: gather from Work0 and Mark0
        //----------------------------------------------------------------------

        if (nthreads == 1)
        {

            //------------------------------------------------------------------
            // gather sparse T using a single thread
            //------------------------------------------------------------------

            int64_t p = 0 ;
            for (int64_t i = 0 ; i < n ; i++)
            {
                if (Mark0 [i])
                { 
                    Ti [p] = i ;
                    // Tx [p] = Work0 [i], no typecast
                    GB_COPY_ARRAY_TO_ARRAY (Tx, p, Work0, i) ;
                    p++ ;
                }
            }
            ASSERT (p == tnz) ;

        }
        else
        {

            //------------------------------------------------------------------
            // gather sparse T using multiple threads
            //------------------------------------------------------------------

            // Some tasks may be completely empty and thus take no time at all;
            // 256 tasks per thread are created for better load balancing.

            int taskid ;
            #pragma omp parallel for num_threads(nthreads) schedule(dynamic)
            for (taskid = 0 ; taskid < ntasks ; taskid++)
            {
                int64_t ifirst, ilast, p = 0 ;
                GB_PARTITION (ifirst, ilast, n, taskid, ntasks) ;
                for (int64_t i = ifirst ; i < ilast ; i++)
                { 
                    p += Mark0 [i] ;
                }
                Count [taskid] = p ;
            }

            GB_cumsum (Count, ntasks, NULL, 1) ;

            #pragma omp parallel for num_threads(nthreads) schedule(dynamic)
            for (taskid = 0 ; taskid < ntasks ; taskid++)
            {
                int64_t ifirst, ilast, p = Count [taskid] ;
                int64_t my_count = (Count [taskid+1] - p) ;
                GB_PARTITION (ifirst, ilast, n, taskid, ntasks) ;
                if (my_count > 0)
                {
                    for (int64_t i = ifirst ; i < ilast ; i++)
                    {
                        if (Mark0 [i])
                        { 
                            Ti [p] = i ;
                            // Tx [p] = Work0 [i], no typecast
                            GB_COPY_ARRAY_TO_ARRAY (Tx, p, Work0, i) ;
                            p++ ;
                        }
                    }
                }
            }

            #ifdef GB_DEBUG
            // check result using a single thread
            int64_t p = 0 ;
            for (int64_t i = 0 ; i < n ; i++)
            {
                if (Mark0 [i])
                {
                    ASSERT (Ti [p] == i) ;
                    p++ ;
                }
            }
            ASSERT (p == tnz) ;
            #endif
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_FREE_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
    GB_FREE_MEMORY (Work0, n, zsize) ;
    GB_FREE_MEMORY (Mark0, n, sizeof (bool)) ;
}


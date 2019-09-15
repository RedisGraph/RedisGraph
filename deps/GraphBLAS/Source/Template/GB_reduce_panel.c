//------------------------------------------------------------------------------
// GB_reduce_panel: s=reduce(A), reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce a matrix to a scalar using a panel-based method for built-in
// operators.  No typecasting is performed.

{

    const GB_ATYPE *restrict Ax = A->x ;
    int64_t anz = GB_NNZ (A) ;
    ASSERT (anz > 0) ;

    if (nthreads == 1)
    {

        //----------------------------------------------------------------------
        // load the Panel with the first entries
        //----------------------------------------------------------------------

        GB_ATYPE Panel [GB_PANEL] ;
        int64_t first_panel_size = GB_IMIN (GB_PANEL, anz) ;
        for (int64_t k = 0 ; k < first_panel_size ; k++)
        {
            Panel [k] = Ax [k] ;
        }

        #if GB_HAS_TERMINAL
        int panel_count = 0 ;
        #endif

        //----------------------------------------------------------------------
        // reduce all entries to the Panel
        //----------------------------------------------------------------------

        for (int64_t p = GB_PANEL ; p < anz ; p += GB_PANEL)
        {
            if (p + GB_PANEL > anz)
            {
                // last partial panel
                for (int64_t k = 0 ; k < anz-p ; k++)
                { 
                    // Panel [k] = op (Panel [k], Ax [p+k]) ;
                    GB_ADD_ARRAY_TO_ARRAY (Panel, k, Ax, p+k) ;
                }
            }
            else
            {
                // full panel
                for (int64_t k = 0 ; k < GB_PANEL ; k++)
                { 
                    // Panel [k] = op (Panel [k], Ax [p+k]) ;
                    GB_ADD_ARRAY_TO_ARRAY (Panel, k, Ax, p+k) ;
                }
                #if GB_HAS_TERMINAL
                panel_count-- ;
                if (panel_count <= 0)
                {
                    // check for early exit only every 256 panels
                    panel_count = 256 ;
                    int count = 0 ;
                    for (int64_t k = 0 ; k < GB_PANEL ; k++)
                    {
                        count += (Panel [k] == GB_TERMINAL_VALUE) ;
                    }
                    if (count > 0)
                    { 
                        break ;
                    }
                }
                #endif
            }
        }

        //----------------------------------------------------------------------
        // s = reduce (Panel)
        //----------------------------------------------------------------------

        s = Panel [0] ;
        for (int64_t k = 1 ; k < first_panel_size ; k++)
        { 
            // s = op (s, Panel [k]) ;
            GB_ADD_ARRAY_TO_SCALAR (s, Panel, k) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // create workspace for multiple threads
        //----------------------------------------------------------------------

        GB_CTYPE W [ntasks] ;
        ASSERT (ntasks <= anz) ;

        //----------------------------------------------------------------------
        // all tasks share a single early_exit flag
        //----------------------------------------------------------------------

        // If this flag gets set, all tasks can terminate early

        #if GB_HAS_TERMINAL
        bool early_exit = false ;
        #endif

        //----------------------------------------------------------------------
        // each thread reduces its own slice in parallel
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (int tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // determine the work for this task
            //------------------------------------------------------------------

            // Task tid reduces Ax [pstart:pend-1] to the scalar W [tid]

            int64_t pstart, pend ;
            GB_PARTITION (pstart, pend, anz, tid, ntasks) ;
            GB_ATYPE t = Ax [pstart] ;

            //------------------------------------------------------------------
            // skip this task if the terminal value has already been reached
            //------------------------------------------------------------------

            #if GB_HAS_TERMINAL
            // check if another task has called for an early exit
            bool my_exit ;
            #pragma omp atomic read
            my_exit = early_exit ;
            if (!my_exit)
            #endif

            //------------------------------------------------------------------
            // do the reductions for this task
            //------------------------------------------------------------------

            {

                //--------------------------------------------------------------
                // load the Panel with the first entries
                //--------------------------------------------------------------

                GB_ATYPE Panel [GB_PANEL] ;
                int64_t my_anz = pend - pstart ;
                int64_t first_panel_size = GB_IMIN (GB_PANEL, my_anz) ;
                for (int64_t k = 0 ; k < first_panel_size ; k++)
                { 
                    Panel [k] = Ax [pstart + k] ;
                }

                #if GB_HAS_TERMINAL
                int panel_count = 0 ;
                #endif

                //--------------------------------------------------------------
                // reduce all entries to the Panel
                //--------------------------------------------------------------

                for (int64_t p = pstart + GB_PANEL ; p < pend ; p += GB_PANEL)
                {
                    if (p + GB_PANEL > pend)
                    {
                        // last partial panel
                        for (int64_t k = 0 ; k < pend-p ; k++)
                        { 
                            // Panel [k] = op (Panel [k], Ax [p+k]) ;
                            GB_ADD_ARRAY_TO_ARRAY (Panel, k, Ax, p+k) ;
                        }
                    }
                    else
                    {
                        // full panel
                        for (int64_t k = 0 ; k < GB_PANEL ; k++)
                        { 
                            // Panel [k] = op (Panel [k], Ax [p+k]) ;
                            GB_ADD_ARRAY_TO_ARRAY (Panel, k, Ax, p+k) ;
                        }
                        #if GB_HAS_TERMINAL
                        panel_count-- ;
                        if (panel_count <= 0)
                        {
                            // check for early exit only every 256 panels
                            panel_count = 256 ;
                            int count = 0 ;
                            for (int64_t k = 0 ; k < GB_PANEL ; k++)
                            {
                                count += (Panel [k] == GB_TERMINAL_VALUE) ;
                            }
                            if (count > 0)
                            { 
                                break ;
                            }
                        }
                        #endif
                    }
                }

                //--------------------------------------------------------------
                // t = reduce (Panel)
                //--------------------------------------------------------------

                t = Panel [0] ;
                for (int64_t k = 1 ; k < first_panel_size ; k++)
                { 
                    // t = op (t, Panel [k]) ;
                    GB_ADD_ARRAY_TO_SCALAR (t, Panel, k) ;
                }

                #if GB_HAS_TERMINAL
                if (t == GB_TERMINAL_VALUE)
                { 
                    // tell all other tasks to exit early
                    #pragma omp atomic write
                    early_exit = true ;
                }
                #endif
            }

            //------------------------------------------------------------------
            // save the results of this task
            //------------------------------------------------------------------

            W [tid] = t ;
        }

        //----------------------------------------------------------------------
        // sum up the results of each slice using a single thread
        //----------------------------------------------------------------------

        s = W [0] ;
        for (int tid = 1 ; tid < ntasks ; tid++)
        { 
            // s = op (s, W [tid]), no typecast
            GB_ADD_ARRAY_TO_SCALAR (s, W, tid) ;
        }
    }
}


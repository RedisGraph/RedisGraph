//------------------------------------------------------------------------------
// GB_AxB_dot3_one_slice: slice the entries and vectors of a single matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Constructs a set of tasks that slice a single input matrix M.  This function
// is currently only used by GB_AxB_dot3, to slice the mask matrix M, which has
// the same pattern as the output matrix C.  However, this function is a very
// simple general-purpose method for slicing a single matrix.  It could be
// called GB_one_slice, and used for other methods as well.

#define GB_FREE_WORK \
    GB_FREE_MEMORY (Coarse, ntasks1+1, sizeof (int64_t)) ;

#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_FREE_WORK ;                                                      \
    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;  \
}

#include "GB_mxm.h"

#define GB_NTASKS_PER_THREAD 256

//------------------------------------------------------------------------------
// GB_AxB_dot3_one_slice
//------------------------------------------------------------------------------

GrB_Info GB_AxB_dot3_one_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads to use
    // input:
    const GrB_Matrix M,             // matrix to slice
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (p_TaskList != NULL) ;
    ASSERT (p_max_ntasks != NULL) ;
    ASSERT (p_ntasks != NULL) ;
    ASSERT (p_nthreads != NULL) ;
    ASSERT_MATRIX_OK (M, "M for dot3_one_slice", GB0) ;

    (*p_TaskList  ) = NULL ;
    (*p_max_ntasks) = 0 ;
    (*p_ntasks    ) = 0 ;
    (*p_nthreads  ) = 1 ;

    //--------------------------------------------------------------------------
    // determine # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Mp = M->p ;
    const int64_t mnz = GB_NNZ (M) ;
    const int64_t mnvec = M->nvec ;

    //--------------------------------------------------------------------------
    // allocate the initial TaskList
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Coarse = NULL ;
    int ntasks1 = 0 ;
    int nthreads = GB_nthreads (mnz, chunk, nthreads_max) ;
    GB_task_struct *GB_RESTRICT TaskList = NULL ;
    int max_ntasks = 0 ;
    int ntasks = 0 ;
    int ntasks0 = (nthreads == 1) ? 1 : (GB_NTASKS_PER_THREAD * nthreads) ;
    GB_REALLOC_TASK_LIST (TaskList, ntasks0, max_ntasks) ;

    //--------------------------------------------------------------------------
    // check for quick return for a single task
    //--------------------------------------------------------------------------

    if (mnvec == 0 || ntasks0 == 1)
    { 
        // construct a single coarse task that does all the work
        TaskList [0].kfirst = 0 ;
        TaskList [0].klast  = mnvec-1 ;
        (*p_TaskList  ) = TaskList ;
        (*p_max_ntasks) = max_ntasks ;
        (*p_ntasks    ) = (mnvec == 0) ? 0 : 1 ;
        (*p_nthreads  ) = 1 ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // determine # of threads and tasks
    //--------------------------------------------------------------------------

    double target_task_size = ((double) mnz) / (double) (ntasks0) ;
    target_task_size = GB_IMAX (target_task_size, chunk) ;
    ntasks1 = ((double) mnz) / target_task_size ;
    ntasks1 = GB_IMAX (ntasks1, 1) ;

    //--------------------------------------------------------------------------
    // slice the work into coarse tasks
    //--------------------------------------------------------------------------

    if (!GB_pslice (&Coarse, Mp, mnvec, ntasks1))
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // construct all tasks, both coarse and fine
    //--------------------------------------------------------------------------

    for (int t = 0 ; t < ntasks1 ; t++)
    {

        //----------------------------------------------------------------------
        // coarse task operates on M (:, k:klast)
        //----------------------------------------------------------------------

        int64_t k = Coarse [t] ;
        int64_t klast = Coarse [t+1] - 1 ;

        if (k >= mnvec)
        { 

            //------------------------------------------------------------------
            // all tasks have been constructed
            //------------------------------------------------------------------

            break ;

        }
        else if (k < klast)
        { 

            //------------------------------------------------------------------
            // coarse task has 2 or more vectors
            //------------------------------------------------------------------

            // This is a non-empty coarse-grain task that does two or more
            // entire vectors of M and C, vectors k:klast, inclusive.
            GB_REALLOC_TASK_LIST (TaskList, ntasks + 1, max_ntasks) ;
            TaskList [ntasks].kfirst = k ;
            TaskList [ntasks].klast  = klast ;
            ntasks++ ;

        }
        else
        {

            //------------------------------------------------------------------
            // coarse task has 0 or 1 vectors
            //------------------------------------------------------------------

            // As a coarse-grain task, this task is empty or does a single
            // vector, k.  Vector k must be removed from the work done by this
            // and any other coarse-grain task, and split into one or more
            // fine-grain tasks.

            for (int tt = t ; tt < ntasks1 ; tt++)
            {
                // remove k from the initial slice tt
                if (Coarse [tt] == k)
                { 
                    // remove k from task tt
                    Coarse [tt] = k+1 ;
                }
                else
                { 
                    // break, k not in task tt
                    break ;
                }
            }

            //------------------------------------------------------------------
            // determine the # of fine-grain tasks to create for vector k
            //------------------------------------------------------------------

            int64_t mknz = Mp [k+1] - Mp [k] ;
            int nfine = ((double) mknz) / target_task_size ;
            nfine = GB_IMAX (nfine, 1) ;

            // make the TaskList bigger, if needed
            GB_REALLOC_TASK_LIST (TaskList, ntasks + nfine, max_ntasks) ;

            //------------------------------------------------------------------
            // create the fine-grain tasks
            //------------------------------------------------------------------

            if (nfine == 1)
            { 

                //--------------------------------------------------------------
                // this is a single coarse task for all of vector k
                //--------------------------------------------------------------

                TaskList [ntasks].kfirst = k ;
                TaskList [ntasks].klast  = k ;
                ntasks++ ;

            }
            else
            {

                //--------------------------------------------------------------
                // slice vector M(:,k) into nfine fine tasks
                //--------------------------------------------------------------

                ASSERT (ntasks < max_ntasks) ;

                for (int tfine = 0 ; tfine < nfine ; tfine++)
                { 

                    // this fine task operates on vector M(:,k)
                    TaskList [ntasks].kfirst = k ;
                    TaskList [ntasks].klast  = -1 ;

                    // slice M(:,k) for this task
                    int64_t p1, p2 ;
                    GB_PARTITION (p1, p2, mknz, tfine, nfine) ;
                    int64_t pM     = Mp [k] + p1 ;
                    int64_t pM_end = Mp [k] + p2 ;
                    TaskList [ntasks].pM     = pM ;
                    TaskList [ntasks].pM_end = pM_end ;

                    ASSERT (TaskList [ntasks].pM <= TaskList [ntasks].pM_end) ;
                    ntasks++ ;
                }
            }
        }
    }

    ASSERT (ntasks <= max_ntasks) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    (*p_TaskList  ) = TaskList ;
    (*p_max_ntasks) = max_ntasks ;
    (*p_ntasks    ) = ntasks ;
    (*p_nthreads  ) = nthreads ;
    return (GrB_SUCCESS) ;
}


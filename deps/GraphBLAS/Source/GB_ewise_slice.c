//------------------------------------------------------------------------------
// GB_ewise_slice: slice the entries and vectors for an ewise operation
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Constructs a set of tasks to compute C, for an element-wise operation
// (GB_add, GB_emult, and GB_mask) that operates on two input matrices,
// C=op(A,B).  The mask is ignored for computing where to slice the work, but
// it is sliced once the location has been found.

#define GB_FREE_WORK                                                    \
{                                                                       \
    GB_FREE_MEMORY (Coarse, ntasks1+1, sizeof (int64_t)) ;              \
    GB_FREE_MEMORY (Cwork, Cnvec+1, sizeof (int64_t)) ;                 \
}

#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_FREE_WORK ;                                                      \
    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;  \
}

#include "GB.h"

//------------------------------------------------------------------------------
// GB_ewise_slice
//------------------------------------------------------------------------------

GrB_Info GB_ewise_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads for eWise operation
    // input:
    const int64_t Cnvec,            // # of vectors of C
    const int64_t *GB_RESTRICT Ch,     // vectors of C, if hypersparse
    const int64_t *GB_RESTRICT C_to_M, // mapping of C to M
    const int64_t *GB_RESTRICT C_to_A, // mapping of C to A
    const int64_t *GB_RESTRICT C_to_B, // mapping of C to B
    bool Ch_is_Mh,                  // if true, then Ch == Mh; GB_add only
    const GrB_Matrix M,             // mask matrix to slice (optional)
    const GrB_Matrix A,             // matrix to slice
    const GrB_Matrix B,             // matrix to slice
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
    ASSERT_MATRIX_OK (A, "A for ewise_slice", GB0) ;
    ASSERT_MATRIX_OK (B, "B for ewise_slice", GB0) ;

    (*p_TaskList  ) = NULL ;
    (*p_max_ntasks) = 0 ;
    (*p_ntasks    ) = 0 ;
    (*p_nthreads  ) = 1 ;

    int64_t *GB_RESTRICT Cwork = NULL ;
    int64_t *GB_RESTRICT Coarse = NULL ; // size ntasks1+1
    int ntasks1 = 0 ;

    //--------------------------------------------------------------------------
    // determine # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // allocate the initial TaskList
    //--------------------------------------------------------------------------

    // Allocate the TaskList to hold at least 2*ntask0 tasks.  It will grow
    // later, if needed.  Usually, 64*nthreads_max is enough, but in a few cases
    // fine tasks can cause this number to be exceeded.  If that occurs,
    // TaskList is reallocated.

    // When the mask is present, it is often fastest to break the work up
    // into tasks, even when nthreads_max is 1.

    GB_task_struct *GB_RESTRICT TaskList = NULL ;
    int max_ntasks = 0 ;
    int ntasks0 = (M == NULL && nthreads_max == 1) ? 1 : (32 * nthreads_max) ;
    GB_REALLOC_TASK_LIST (TaskList, ntasks0, max_ntasks) ;

    //--------------------------------------------------------------------------
    // check for quick return for a single task
    //--------------------------------------------------------------------------

    if (Cnvec == 0 || ntasks0 == 1)
    { 
        // construct a single coarse task that computes all of C
        TaskList [0].kfirst = 0 ;
        TaskList [0].klast  = Cnvec-1 ;
        (*p_TaskList  ) = TaskList ;
        (*p_max_ntasks) = max_ntasks ;
        (*p_ntasks    ) = (Cnvec == 0) ? 0 : 1 ;
        (*p_nthreads  ) = 1 ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // get A, B, and M
    //--------------------------------------------------------------------------

    const int64_t vlen = A->vlen ;
    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    bool Ch_is_Ah = (Ch != NULL && A->h != NULL && Ch == A->h) ;
    bool Ch_is_Bh = (Ch != NULL && B->h != NULL && Ch == B->h) ;

    const int64_t *GB_RESTRICT Mp = NULL ;
    const int64_t *GB_RESTRICT Mi = NULL ;
    if (M != NULL)
    { 
        Mp = M->p ;
        Mi = M->i ;
        // Ch_is_Mh is true if either true on input (for GB_add, which denotes
        // that Ch is a deep copy of M->h), or if Ch is a shallow copy of M->h.
        Ch_is_Mh = Ch_is_Mh || (Ch != NULL && M->h != NULL && Ch == M->h) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    GB_MALLOC_MEMORY (Cwork, Cnvec+1, sizeof (int64_t)) ;
    if (Cwork == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // compute an estimate of the work for each vector of C
    //--------------------------------------------------------------------------

    int nthreads_for_Cwork = GB_nthreads (Cnvec, chunk, nthreads_max) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads_for_Cwork) schedule(static)
    for (k = 0 ; k < Cnvec ; k++)
    {

        //----------------------------------------------------------------------
        // get the C(:,j) vector
        //----------------------------------------------------------------------

        int64_t j = (Ch == NULL) ? k : Ch [k] ;

        //----------------------------------------------------------------------
        // get the corresponding vector of A
        //----------------------------------------------------------------------

        int64_t kA ;
        if (C_to_A != NULL)
        { 
            // A is hypersparse and the C_to_A mapping has been created
            ASSERT (A->is_hyper || A->is_slice) ;
            kA = C_to_A [k] ;
            ASSERT (kA >= -1 && kA < A->nvec) ;
            if (kA >= 0)
            {
                ASSERT (j == ((A->is_hyper) ? A->h [kA] : (A->hfirst + kA))) ;
            }
        }
        else if (Ch_is_Ah)
        { 
            // A is hypersparse, but Ch is a shallow copy of A->h
            kA = k ;
            ASSERT (j == A->h [kA]) ;
        }
        else
        { 
            // A is standard
            ASSERT (!A->is_hyper) ;
            ASSERT (!A->is_slice) ;
            ASSERT (A->h == NULL) ;
            kA = j ;
        }

        //----------------------------------------------------------------------
        // get the corresponding vector of B
        //----------------------------------------------------------------------

        int64_t kB ;
        if (C_to_B != NULL)
        { 
            // B is hypersparse and the C_to_B mapping has been created
            ASSERT (B->is_hyper || B->is_slice) ;
            kB = C_to_B [k] ;
            ASSERT (kB >= -1 && kB < B->nvec) ;
            if (kB >= 0)
            {
                ASSERT (j == ((B->is_hyper) ? B->h [kB] : (B->hfirst + kB))) ;
            }
        }
        else if (Ch_is_Bh)
        { 
            // B is hypersparse, but Ch is a shallow copy of B->h
            kB = k ;
            ASSERT (j == B->h [kB]) ;
        }
        else
        { 
            // B is standard
            ASSERT (!B->is_hyper) ;
            ASSERT (!B->is_slice) ;
            ASSERT (B->h == NULL) ;
            kB = j ;
        }

        //----------------------------------------------------------------------
        // estimate the work for C(:,j)
        //----------------------------------------------------------------------

        ASSERT (kA >= -1 && kA < A->nvec) ;
        ASSERT (kB >= -1 && kB < B->nvec) ;
        int64_t aknz = (kA < 0) ? 0 : (Ap [kA+1] - Ap [kA]) ;
        int64_t bknz = (kB < 0) ? 0 : (Bp [kB+1] - Bp [kB]) ;

        Cwork [k] = aknz + bknz + 1 ;
    }

    //--------------------------------------------------------------------------
    // replace Cwork with its cumulative sum
    //--------------------------------------------------------------------------

    GB_cumsum (Cwork, Cnvec, NULL, nthreads_for_Cwork) ;
    double cwork = (double) Cwork [Cnvec] ;

    //--------------------------------------------------------------------------
    // determine # of threads and tasks for the eWise operation
    //--------------------------------------------------------------------------

    int nthreads = GB_nthreads (cwork, chunk, nthreads_max) ;

    ntasks0 = (M == NULL && nthreads == 1) ? 1 : (32 * nthreads) ;
    double target_task_size = cwork / (double) (ntasks0) ;
    target_task_size = GB_IMAX (target_task_size, chunk) ;
    ntasks1 = cwork / target_task_size ;
    ntasks1 = GB_IMAX (ntasks1, 1) ;

    //--------------------------------------------------------------------------
    // slice the work into coarse tasks
    //--------------------------------------------------------------------------

    if (!GB_pslice (&Coarse, Cwork, Cnvec, ntasks1))
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // construct all tasks, both coarse and fine
    //--------------------------------------------------------------------------

    int ntasks = 0 ;

    for (int t = 0 ; t < ntasks1 ; t++)
    {

        //----------------------------------------------------------------------
        // coarse task computes C (:,k:klast)
        //----------------------------------------------------------------------

        int64_t k = Coarse [t] ;
        int64_t klast  = Coarse [t+1] - 1 ;

        if (k >= Cnvec)
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
            // entire vectors of C, vectors k:klast, inclusive.
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
            // get the vector of C
            //------------------------------------------------------------------

            int64_t j = (Ch == NULL) ? k : Ch [k] ;

            //------------------------------------------------------------------
            // get the corresponding vector of A
            //------------------------------------------------------------------

            int64_t kA ;
            if (C_to_A != NULL)
            { 
                // A is hypersparse and the C_to_A mapping has been created
                kA = C_to_A [k] ;
            }
            else if (Ch_is_Ah)
            { 
                // A is hypersparse, but Ch is a shallow copy of A->h
                kA = k ;
            }
            else
            { 
                // A is standard
                kA = j ;
            }
            int64_t pA_start = (kA < 0) ? -1 : Ap [kA] ;
            int64_t pA_end   = (kA < 0) ? -1 : Ap [kA+1] ;
            bool a_empty = (pA_end == pA_start) ;

            //------------------------------------------------------------------
            // get the corresponding vector of B
            //------------------------------------------------------------------

            int64_t kB ;
            if (C_to_B != NULL)
            { 
                // B is hypersparse and the C_to_B mapping has been created
                kB = C_to_B [k] ;
            }
            else if (Ch_is_Bh)
            { 
                // B is hypersparse, but Ch is a shallow copy of B->h
                kB = k ;
            }
            else
            { 
                // B is standard
                kB = j ;
            }
            int64_t pB_start = (kB < 0) ? -1 : Bp [kB] ;
            int64_t pB_end   = (kB < 0) ? -1 : Bp [kB+1] ;
            bool b_empty = (pB_end == pB_start) ;

            //------------------------------------------------------------------
            // get the corresponding vector of M, if present
            //------------------------------------------------------------------

            int64_t pM_start = -1 ;
            int64_t pM_end   = -1 ;
            if (M != NULL)
            {
                int64_t kM ;
                if (C_to_M != NULL)
                { 
                    // M is hypersparse and the C_to_M mapping has been created
                    kM = C_to_M [k] ;
                }
                else if (Ch_is_Mh)
                {
                    // Ch is a deep or shallow copy of Mh
                    kM = k ;
                }
                else
                { 
                    // M is standard
                    kM = j ;
                }
                pM_start = (kM < 0) ? -1 : Mp [kM] ;
                pM_end   = (kM < 0) ? -1 : Mp [kM+1] ;
            }
            bool m_empty = (pM_end == pM_start) ;

            //------------------------------------------------------------------
            // determine the # of fine-grain tasks to create for vector k
            //------------------------------------------------------------------

            double ckwork = Cwork [k+1] - Cwork [k] ;
            int nfine = ckwork / target_task_size ;
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
                // slice vector k into nfine fine tasks
                //--------------------------------------------------------------

                // first fine task starts at the top of vector k
                ASSERT (ntasks < max_ntasks) ;
                TaskList [ntasks].kfirst = k ;
                TaskList [ntasks].klast  = -1 ; // this is a fine task
                TaskList [ntasks].pM = (m_empty) ? -1 : pM_start ;
                TaskList [ntasks].pA = (a_empty) ? -1 : pA_start ;
                TaskList [ntasks].pB = (b_empty) ? -1 : pB_start ;
                TaskList [ntasks].len = 0 ;     // to be determined below
                ntasks++ ;
                int64_t ilast = 0, i = 0 ;

                for (int tfine = 1 ; tfine < nfine ; tfine++)
                { 
                    double target_work = ((nfine-tfine) * ckwork) / nfine ;
                    int64_t pM, pA, pB ;
                    GB_slice_vector (&i, &pM, &pA, &pB,
                        pM_start, pM_end, Mi,       // Mi NULL if M not present
                        pA_start, pA_end, Ai, 0,    // Ai always explicit list
                        pB_start, pB_end, Bi,       // Bi always explicit list
                        vlen, target_work) ;

                    // prior task ends at pM-1, pA-1, and pB-1
                    TaskList [ntasks-1].pM_end = pM ;
                    TaskList [ntasks-1].pA_end = pA ;
                    TaskList [ntasks-1].pB_end = pB ;

                    // prior task handles indices ilast:i-1
                    TaskList [ntasks-1].len = i - ilast ;

                    // this task starts at pM, pA, and pB 
                    ASSERT (ntasks < max_ntasks) ;
                    TaskList [ntasks].kfirst = k ;
                    TaskList [ntasks].klast  = -1 ; // this is a fine task
                    TaskList [ntasks].pM = pM ;
                    TaskList [ntasks].pA = pA ;
                    TaskList [ntasks].pB = pB ;

                    // advance to the next task
                    ntasks++ ;
                    ilast = i ;
                }

                // Terminate the last fine task.
                ASSERT (ntasks <= max_ntasks) ;
                TaskList [ntasks-1].pM_end = (m_empty) ? -1 : pM_end ;
                TaskList [ntasks-1].pA_end = (a_empty) ? -1 : pA_end ;
                TaskList [ntasks-1].pB_end = (b_empty) ? -1 : pB_end ;
                TaskList [ntasks-1].len = vlen - i ;
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


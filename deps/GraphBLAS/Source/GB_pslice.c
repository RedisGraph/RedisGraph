//------------------------------------------------------------------------------
// GB_pslice: partition Ap for a set of tasks
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Ap [0..n] is an array with monotonically increasing entries.  This function
// slices Ap so that each chunk has the same number of total values of its
// entries.  Ap can be A->p for a matrix and then n = A->nvec.  Or it can be
// the work needed for computing each vector of a matrix (see GB_ewise_slice
// and GB_subref_slice, for example).

// If Ap is NULL then the matrix A (not provided here) is full or bitmap,
// which this function handles (Ap is implicit).

#include "GB.h"

//------------------------------------------------------------------------------
// GB_pslice_worker: partition Ap for a set of tasks
//------------------------------------------------------------------------------

static void GB_pslice_worker
(
    int64_t *GB_RESTRICT Slice,     // size ntasks+1
    const int64_t *GB_RESTRICT Ap,  // array size n+1
    int tlo,                        // assign to Slice [(tlo+1):(thi-1)]
    int thi                     
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    ASSERT (Ap != NULL) ;
    ASSERT (Slice != NULL) ;
    ASSERT (0 <= tlo && tlo < thi - 1) ;
    for (int t = tlo+1 ; t <= thi-1 ; t++)
    {
        ASSERT (Slice [t] == -1) ;
    }
    #endif

    //--------------------------------------------------------------------------
    // assign work to Slice [(tlo+1):(thi-1)]
    //--------------------------------------------------------------------------

    // klo = Slice [tlo] and khi = Slice [thi] are defined on input, where
    // tlo < thi - 1.  This determines the task boundaries for tasks
    // tlo+1 to thi-1, which defines Slice [(tlo+1):(thi-1)].

    int64_t klo = Slice [tlo] ;
    int64_t khi = Slice [thi] ;         ASSERT (0 <= klo && klo <= khi) ;
    int64_t p1 = Ap [klo] ;
    int64_t p2 = Ap [khi] ;             ASSERT (p1 <= p2) ;

    if (p1 == p2 || klo == khi)
    {

        //----------------------------------------------------------------------
        // no work is left so simply fill in with empty tasks
        //----------------------------------------------------------------------

        int64_t k = klo ;
        for (int64_t t = tlo+1 ; t <= thi-1 ; t++)
        { 
            Slice [t] = k ;
        }

    }
    else // p1 < p2 && klo < khi
    {

        //----------------------------------------------------------------------
        // find task t that evenly partitions the work p1:p2 to tasks tlo:thi
        //----------------------------------------------------------------------

        int64_t k = (klo + khi) / 2 ;       ASSERT (klo <= k && k <= khi) ;
        int64_t p = Ap [k] ;                ASSERT (p1 <= p && p <= p2) ;
        double ntasks = thi - tlo ;
        double ratio = (((double) (p - p1)) / ((double) (p2 - p1))) ;
        int t = tlo + (int) floor (ratio * ntasks) ;
        t = GB_IMAX (t, tlo+1) ;
        t = GB_IMIN (t, thi-1) ;            ASSERT (tlo < t && t < thi) ;

        //----------------------------------------------------------------------
        // assign work to task t
        //----------------------------------------------------------------------

        ASSERT (Slice [t] == -1) ;
        Slice [t] = k ;

        //----------------------------------------------------------------------
        // recursively partition for tasks (tlo+1):(t-1) and (t+1):(thi-1)
        //----------------------------------------------------------------------

        if (tlo < t-1)
        { 
            GB_pslice_worker (Slice, Ap, tlo, t) ;
        }
        if (t < thi-1)
        { 
            GB_pslice_worker (Slice, Ap, t, thi) ;
        }
    }
}

//------------------------------------------------------------------------------
// GB_pslice: partition Ap for a set of tasks
//------------------------------------------------------------------------------


GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_pslice          // slice Ap; return true if ok, false if out of memory
(
    int64_t *GB_RESTRICT *Slice_handle,    // size ntasks+1
    const int64_t *GB_RESTRICT Ap,  // array size n+1 (NULL if full or bitmap)
    const int64_t n,
    const int ntasks,               // # of tasks
    const bool perfectly_balanced
)
{

    //--------------------------------------------------------------------------
    // allocate result, unless it is already allocated on input
    //--------------------------------------------------------------------------

    int64_t *Slice ;
    if ((*Slice_handle) == NULL)
    {
        (*Slice_handle) = NULL ;
        Slice = GB_MALLOC (ntasks+1, int64_t) ;
        if (Slice == NULL)
        { 
            // out of memory
            return (false) ;
        }
        (*Slice_handle) = Slice ;
    }
    else
    { 
        Slice = (*Slice_handle) ;
    }

    #ifdef GB_DEBUG
    for (int taskid = 0 ; taskid <= ntasks ; taskid++)
    {
        Slice [taskid] = -1 ;
    }
    #endif

    //--------------------------------------------------------------------------
    // assign first and last task boundaries
    //--------------------------------------------------------------------------

    Slice [0] = 0 ;
    Slice [ntasks] = n ;

    //--------------------------------------------------------------------------
    // slice the work for remainder of the tasks, 1:ntasks-1
    //--------------------------------------------------------------------------

    if (Ap == NULL)
    {

        //----------------------------------------------------------------------
        // A is full or bitmap
        //----------------------------------------------------------------------

        for (int taskid = 1 ; taskid < ntasks ; taskid++)
        { 
            Slice [taskid] = (int64_t) GB_PART (taskid, n, ntasks) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // A is sparse or hypersparse
        //----------------------------------------------------------------------

        if (n == 0 || ntasks <= 1 || Ap [n] == 0)
        {
            // matrix is empty, or a single thread is used
            for (int taskid = 1 ; taskid < ntasks ; taskid++)
            { 
                // slice sparse/hyper with 1 task, n == 0, or no work
                Slice [taskid] = 0 ;
            }
        }
        else
        {
            // slice Ap by # of entries
            if (perfectly_balanced)
            {
                // this method is costly, and should only be used if the
                // work is to be perfectly balanced (in particular, when there
                // is just one task per thread, with static scheduling)
                const double work = (double) (Ap [n]) ;
                int64_t k = 0 ;
                for (int taskid = 1 ; taskid < ntasks ; taskid++)
                { 
                    // binary search to find k so that Ap [k] == (taskid*work) /
                    // ntasks.  The exact value will not typically not be found;
                    // just pick what the binary search comes up with.
                    int64_t wtask = (int64_t) GB_PART (taskid, work, ntasks) ;
                    int64_t pright = n ;
                    GB_TRIM_BINARY_SEARCH (wtask, Ap, k, pright) ;
                    Slice [taskid] = k ;
                }
            }
            else
            { 
                // this is much faster, and results in good load balancing if
                // there is more than one task per thread, and dynamic
                // scheduling is used.
                GB_pslice_worker (Slice, Ap, 0, ntasks) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // check result
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    ASSERT (Slice [0] == 0) ;
    ASSERT (Slice [ntasks] == n) ;
    for (int taskid = 0 ; taskid < ntasks ; taskid++)
    {
        ASSERT (Slice [taskid] <= Slice [taskid+1]) ;
    }
    #endif

    return (true) ;
}


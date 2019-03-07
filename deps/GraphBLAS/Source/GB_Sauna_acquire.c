//------------------------------------------------------------------------------
// GB_Sauna_acquire: acquire a set of Saunas
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If the user only calls GraphBLAS from a single user thread, then all
// internal threads will always find their native Sauna.

// FUTURE: time spent in the critical section is typically O(nthreads) or
// O(GB_Global.nthreads_max), unless the user calls GraphBLAS simultaneously
// from multiple user threads.  In that case, the time could be
// O(GxB_NTHREADS_MAX), which is 2048 by default.  If it becomes a performance
// bottleneck, doubly-linked list of available Sauna ids could be kept,

#include "GB.h"

GrB_Info GB_Sauna_acquire
(
    int nthreads,           // number of internal threads that need a Sauna
    int *Sauna_ids,         // size nthreads, the Sauna id's acquired
    GrB_Desc_Value *AxB_methods_used,       // size nthreads
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Sauna_ids != NULL) ;
    ASSERT (nthreads >= 1) ;

    //--------------------------------------------------------------------------
    // acquire nthread ids of unused Saunas
    //--------------------------------------------------------------------------

    bool ok = true ;
    bool try_again = false ;

    // define the work to do inside the critical section
    #define GB_CRITICAL_SECTION                                             \
    {                                                                       \
        /* try to acquire the native Saunas for each thread */              \
        for (int t = 0 ; t < nthreads ; t++)                                \
        {                                                                   \
            if (AxB_methods_used [t] != GxB_AxB_GUSTAVSON)                  \
            {                                                               \
                /* no need for a Sauna for this thread */                   \
                Sauna_ids [t] = -2 ;                                        \
            }                                                               \
            else if (GB_Global.Sauna_in_use [t])                            \
            {                                                               \
                /* Saunas [t] is already in use */                          \
                try_again = true ;                                          \
                Sauna_ids [t] = -1 ;                                        \
            }                                                               \
            else                                                            \
            {                                                               \
                /* acquire the native Sauna */                              \
                GB_Global.Sauna_in_use [t] = true ;                         \
                Sauna_ids [t] = t ;                                         \
            }                                                               \
        }                                                                   \
        if (try_again)                                                      \
        {                                                                   \
            /* look for non-native Saunas for the unsatisfied threads */    \
            int s = 0 ;                                                     \
            for (int t = 0 ; t < nthreads ; t++)                            \
            {                                                               \
                if (Sauna_ids [t] == -1)                                    \
                {                                                           \
                    /* thread t does not yet have a Sauna */                \
                    for ( ; s < GxB_NTHREADS_MAX ; s++)                     \
                    {                                                       \
                        if (!GB_Global.Sauna_in_use [s])                    \
                        {                                                   \
                            /* acquire the native Sauna */                  \
                            GB_Global.Sauna_in_use [s] = true ;             \
                            Sauna_ids [t] = s ;                             \
                            break ;                                         \
                        }                                                   \
                    }                                                       \
                }                                                           \
            }                                                               \
        }                                                                   \
    }

    //--------------------------------------------------------------------------
    // do the critical section, depending on user threading model
    //--------------------------------------------------------------------------

    #include "GB_critical_section.c"

    if (!ok) return (GrB_PANIC) ;       // critical section failed!

    //--------------------------------------------------------------------------
    // check if all threads got a Sauna that need one
    //--------------------------------------------------------------------------

    for (int t = 0 ; t < nthreads ; t++)
    {
        if (Sauna_ids [t] == -1)
        {
            // release all Sauna ids just acquired
            GrB_Info info = GB_Sauna_release (nthreads, Sauna_ids) ;
            if (info != GrB_SUCCESS) return (info) ;
            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                "Too many concurrent threads"))) ;
        }
    }

    return (GrB_SUCCESS) ;
}


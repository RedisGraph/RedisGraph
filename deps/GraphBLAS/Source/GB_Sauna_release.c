//------------------------------------------------------------------------------
// GB_Sauna_release: release a set of Saunas
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Sauna_release
(
    int nthreads,           // number of internal threads that have a Sauna
    int *Sauna_ids          // size nthreads, the Sauna id's to release
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Sauna_ids != NULL) ;
    ASSERT (nthreads >= 1) ;
    for (int t = 0 ; t < nthreads ; t++)
    {
        ASSERT (Sauna_ids [t] >= -2 && Sauna_ids [t] < GxB_NTHREADS_MAX) ;
    }

    //--------------------------------------------------------------------------
    // release the Saunas
    //--------------------------------------------------------------------------

    bool ok = true ;

    // define the work to do inside the critical section
    #define GB_CRITICAL_SECTION                                             \
    {                                                                       \
        for (int t = 0 ; t < nthreads ; t++)                                \
        {                                                                   \
            int Sauna_id = Sauna_ids [t] ;                                  \
            if (Sauna_id >= 0)                                              \
            {                                                               \
                /* release the Sauna previously acquired for thread t */    \
                GB_Global.Sauna_in_use [Sauna_id] = false ;                 \
            }                                                               \
        }                                                                   \
    }

    //--------------------------------------------------------------------------
    // do the critical section, depending on user threading model
    //--------------------------------------------------------------------------

    #include "GB_critical_section.c"

    // GrB_PANIC if the critical section fails
    return (ok ? GrB_SUCCESS : GrB_PANIC) ;
}


//------------------------------------------------------------------------------
// GB_Sauna_release: release a set of Saunas
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A set of threads has finished computing C=A*B, and can now release their
// thread-specific Sauna workspaces.  This does not need to be done in a
// critical section, but doing so keeps the Sauna acquire/release in a single
// batch, for each group of threads.  Another user thread can start C=A*B just
// as these Saunas are being released, which would lead to an intermingled set
// of Saunas.  It would work just fine, but might not be optimal.  The critical
// section is very short.

#include "GB_Sauna.h"

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
    for (int tid = 0 ; tid < nthreads ; tid++)
    {
        // printf ("tid %d releasing %d\n", tid, Sauna_ids [tid]) ; 
        ASSERT (Sauna_ids [tid] >= -2 && Sauna_ids [tid] < GxB_NTHREADS_MAX) ;
    }

    //--------------------------------------------------------------------------
    // release the Saunas
    //--------------------------------------------------------------------------

    bool ok = true ;

    // define the work to do inside the critical section
    #define GB_CRITICAL_SECTION                                             \
    {                                                                       \
        for (int tid = 0 ; tid < nthreads ; tid++)                          \
        {                                                                   \
            int Sauna_id = Sauna_ids [tid] ;                                \
            if (Sauna_id >= 0)                                              \
            {                                                               \
                /* release the Sauna previously acquired for thread tid */  \
                GB_Global_Sauna_in_use_set (Sauna_id, false) ;              \
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


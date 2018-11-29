//------------------------------------------------------------------------------
// GB_queue_init:  initialize the global matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_queue_init              // initialize the queue
(
    const GrB_Mode mode,        // blocking or non-blocking mode
    bool *p_I_was_first         // true if this is the first time
)
{

    //--------------------------------------------------------------------------
    // clear the queue and initialize the mode
    //--------------------------------------------------------------------------

    bool I_was_first = false ;
    bool ok = true ;

    // clear the queue of matrices for nonblocking mode and set the mode.  The
    // queue must be protected and can be initialized only once by any thread.

    // define the work to do inside the critical section
    #define GB_CRITICAL_SECTION                                         \
    {                                                                   \
        if (!GB_Global.GrB_init_called)                                 \
        {                                                               \
            I_was_first = true ;                                        \
            /* clear the queue */                                       \
            GB_Global.queue_head = NULL ;                               \
            /* set the mode: blocking or nonblocking */                 \
            GB_Global.mode = mode ; /* default is non-blocking mode */  \
            /* first thread has called GrB_init */                      \
            GB_Global.GrB_init_called = true ;                          \
        }                                                               \
    }

    // do the critical section, depending on user threading model
    #include "GB_critical_section.c"

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*p_I_was_first) = I_was_first ;
    return (ok) ;
}


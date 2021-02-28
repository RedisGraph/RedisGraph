//------------------------------------------------------------------------------
// GB_queue_remove: remove a matrix from the matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_queue_remove            // remove matrix from queue
(
    GrB_Matrix A                // matrix to remove
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // remove the matrix from the queue, if it is in the queue
    //--------------------------------------------------------------------------

    bool ok = true ;

    if (A->enqueued)
    { 
        // remove the matrix from the queue

        // define the work to do inside the critical section
        #define GB_CRITICAL_SECTION                                         \
        {                                                                   \
            /* check again to be safe, and remove A from the queue */       \
            if (A->enqueued)                                                \
            {                                                               \
                GrB_Matrix Prev = (GrB_Matrix) (A->queue_prev) ;            \
                GrB_Matrix Next = (GrB_Matrix) (A->queue_next) ;            \
                if (Prev == NULL)                                           \
                {                                                           \
                    /* matrix is at head of the queue; update the head */   \
                    GB_Global_queue_head_set (Next) ;                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* matrix is not the first in the queue */              \
                    Prev->queue_next = Next ;                               \
                }                                                           \
                if (Next != NULL)                                           \
                {                                                           \
                    /* update previous link of the next matrix, if any */   \
                    Next->queue_prev = Prev ;                               \
                }                                                           \
                /* A has been removed from the queue */                     \
                A->queue_prev = NULL ;                                      \
                A->queue_next = NULL ;                                      \
                A->enqueued = false ;                                       \
            }                                                               \
        }

        // do the critical section, depending on user threading model
        #include "GB_critical_section.c"
    }

    return (ok) ;
}


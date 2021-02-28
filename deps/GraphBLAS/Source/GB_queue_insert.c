//------------------------------------------------------------------------------
// GB_queue_insert:  insert a matrix at the head of the matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check if the matrix has pending computations (either pending tuples or
// zombies, or both).  If it has any, and if it is not already in the queue,
// then insert it into the queue.

#include "GB.h"

bool GB_queue_insert            // insert matrix at the head of queue
(
    GrB_Matrix A                // matrix to insert
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // insert the matrix at the head of the queue
    //--------------------------------------------------------------------------

    bool ok = true ;

    if ((A->Pending != NULL || A->nzombies > 0) && !(A->enqueued))
    { 
        // A is not in the queue yet, but needs to be there

        // define the work to do inside the critical section
        #define GB_CRITICAL_SECTION                                         \
        {                                                                   \
            /* check again to be safe, then add A to the head of queue */   \
            if ((A->Pending != NULL || A->nzombies > 0) && !(A->enqueued))  \
            {                                                               \
                /* add the matrix to the head of the queue */               \
                GrB_Matrix Head = (GrB_Matrix) (GB_Global_queue_head_get ( )) ;\
                A->queue_next = Head ;                                      \
                A->queue_prev = NULL ;                                      \
                A->enqueued = true ;                                        \
                if (Head != NULL)                                           \
                {                                                           \
                    Head->queue_prev = A ;                                  \
                }                                                           \
                GB_Global_queue_head_set (A) ;                              \
            }                                                               \
        }

        // do the critical section, depending on user threading model
        #include "GB_critical_section.c"

    }

    return (ok) ;
}


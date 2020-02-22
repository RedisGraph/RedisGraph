//------------------------------------------------------------------------------
// GB_queue_remove_head: remove the matrix at the head of the matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_queue_remove_head       // remove matrix at the head of queue
(
    GrB_Matrix *Ahandle         // return matrix or NULL if queue empty
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Ahandle != NULL) ;

    //--------------------------------------------------------------------------
    // remove the matrix at the head the queue
    //--------------------------------------------------------------------------

    GrB_Matrix A = NULL ;
    bool ok = true ;

    // define the work to do inside the critical section
    #define GB_CRITICAL_SECTION                                             \
    {                                                                       \
        /* get the matrix at the head of the queue */                       \
        A = (GrB_Matrix) (GB_Global_queue_head_get ( )) ;                   \
        /* remove A from the queue, if it exists */                         \
        if (A != NULL)                                                      \
        {                                                                   \
            ASSERT (A->enqueued) ;                                          \
            ASSERT (A->queue_prev == NULL) ;                                \
            /* shift the head to the next matrix in the queue */            \
            GrB_Matrix Next = (GrB_Matrix) A->queue_next ;                  \
            GB_Global_queue_head_set (Next) ;                               \
            if (Next != NULL)                                               \
            {                                                               \
                Next->queue_prev = NULL ;                                   \
            }                                                               \
            /* A has been removed from the queue */                         \
            A->queue_next = NULL ;                                          \
            A->enqueued = false ;                                           \
        }                                                                   \
    }

    // do the critical section, depending on user threading model
    #include "GB_critical_section.c"

    //--------------------------------------------------------------------------
    // return the matrix that was just removed from the head the queue
    //--------------------------------------------------------------------------

    (*Ahandle) = A ;
    return (ok) ;
}


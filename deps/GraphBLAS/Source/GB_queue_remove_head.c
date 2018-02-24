//------------------------------------------------------------------------------
// GB_queue_remove_head: remove the matrix at the head of the matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Matrix GB_queue_remove_head ( )   // return matrix or NULL if queue empty
{

    //--------------------------------------------------------------------------
    // remove the matrix at the head the queue
    //--------------------------------------------------------------------------

    GrB_Matrix A = NULL ;

    #pragma omp critical (GB_queue)
    {
        // get the matrix at the head of the queue
        A = (GrB_Matrix) (GB_Global.queue_head) ;
        // remove A from the queue, if it exists
        if (A != NULL)
        {
            ASSERT (A->enqueued) ;
            ASSERT (A->queue_prev == NULL) ;
            // shift the head to the next matrix in the queue
            GrB_Matrix Next = (GrB_Matrix) A->queue_next ;
            GB_Global.queue_head = Next ;
            if (Next != NULL)
            {
                Next->queue_prev = NULL ;
            }
            // A has been removed from the queue
            A->queue_next = NULL ;
            A->enqueued = false ;
        }
    }

    //--------------------------------------------------------------------------
    // return the matrix that was just removed from the head the queue
    //--------------------------------------------------------------------------

    return (A) ;
}


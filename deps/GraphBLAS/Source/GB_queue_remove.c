//------------------------------------------------------------------------------
// GB_queue_remove: remove a matrix from the matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_queue_remove            // remove matrix from queue
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

    if (A->enqueued)
    {
        // remove the matrix from the queue
        #pragma omp critical (GB_queue)
        {
            // check again to be safe, and remove A from the queue
            if (A->enqueued)
            {
                GrB_Matrix Prev = (GrB_Matrix) (A->queue_prev) ;
                GrB_Matrix Next = (GrB_Matrix) (A->queue_next) ;
                if (Prev == NULL)
                {
                    // matrix is at the head of the queue; update the head
                    GB_Global.queue_head = Next ;
                }
                else
                {
                    // matrix is not the first in the queue
                    Prev->queue_next = Next ;
                }
                if (Next != NULL)
                {
                    // update the previous link of the next matrix, if any
                    Next->queue_prev = Prev ;
                }
                // A has been removed from the queue
                A->queue_prev = NULL ;
                A->queue_next = NULL ;
                A->enqueued = false ;
            }
        }
    }
}


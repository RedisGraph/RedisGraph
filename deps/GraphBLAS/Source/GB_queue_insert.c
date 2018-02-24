//------------------------------------------------------------------------------
// GB_queue_insert:  insert a matrix at the head of the matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check if the matrix has pending computations (either pending tuples or
// zombies, or both).  If it has any, and if it is not already in the queue,
// then insert it into the queue.

#include "GB.h"

void GB_queue_insert            // insert matrix at the head of queue
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

    if ((A->npending > 0 || A->nzombies > 0) && !(A->enqueued))
    {
        // A is not in the queue yet, but needs to be there
        #pragma omp critical (GB_queue)
        {
            // check again to be safe, then add A to the head of the queue
            if ((A->npending > 0 || A->nzombies > 0) && !(A->enqueued))
            {
                // add the matrix to the head of the queue
                GrB_Matrix Head = (GrB_Matrix) (GB_Global.queue_head) ;
                A->queue_next = Head ;
                A->queue_prev = NULL ;
                A->enqueued = true ;
                if (Head != NULL)
                {
                    Head->queue_prev = A ;
                }
                GB_Global.queue_head = A ;
            }
        }
    }
}


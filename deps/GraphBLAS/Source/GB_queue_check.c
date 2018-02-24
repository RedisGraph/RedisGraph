//------------------------------------------------------------------------------
// GB_queue_check:  check the status of the queue for a particular matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_queue_check
(
    GrB_Matrix A,           // matrix to check
    GrB_Matrix *head,       // head of the queue
    GrB_Matrix *prev,       // prev from A
    GrB_Matrix *next,       // next after A
    bool *enqd              // true if A is in the queue
)
{

    #pragma omp critical (GB_queue)
    {
        // get the status of the queue for this matrix
        (*head) = (GrB_Matrix) (GB_Global.queue_head) ;
        (*prev) = (GrB_Matrix) (A->queue_prev) ;
        (*next) = (GrB_Matrix) (A->queue_next) ;
        (*enqd) = A->enqueued ;
    }
}


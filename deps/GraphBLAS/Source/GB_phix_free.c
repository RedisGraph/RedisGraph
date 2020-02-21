//------------------------------------------------------------------------------
// GB_phix_free: free all content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Frees all allocatable content of a matrix, except for the header itself.
// A->magic becomes GB_MAGIC2.  If this matrix is given to a user-callable
// GraphBLAS function, it will generate a GrB_INVALID_OBJECT error.

// This function normally returns GrB_SUCCESS. 

// If A is in the queue, it is removed, which requires access to the critical
// section for the queue.  Normally this cannot fail, but In the rare event
// that the critical section does fail, a GrB_PANIC is returned.


#include "GB.h"

GrB_Info GB_phix_free           // free all content of a matrix
(
    GrB_Matrix A                // handle of matrix with content to free
)
{ 

    GB_ph_free (A) ;
    return (GB_ix_free (A)) ;   // GrB_SUCCESS or GrB_PANIC
}


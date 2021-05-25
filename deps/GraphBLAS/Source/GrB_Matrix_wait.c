//------------------------------------------------------------------------------
// GrB_Matrix_wait: wait for a matrix to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

#define GB_FREE_ALL ;

GrB_Info GrB_Matrix_wait    // finish all work on a matrix
(
    GrB_Matrix *A
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ((*A), "GrB_Matrix_wait (&A)") ;
    GB_RETURN_IF_NULL (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*A) ;

    //--------------------------------------------------------------------------
    // finish all pending work on the matrix
    //--------------------------------------------------------------------------

    if (GB_ANY_PENDING_WORK (*A))
    { 
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Matrix_wait") ;
        GB_OK (GB_Matrix_wait (*A, Context)) ;
        GB_BURBLE_END ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}


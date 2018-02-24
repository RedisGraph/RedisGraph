//------------------------------------------------------------------------------
// GrB_Matrix_dup: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A, making a deep copy

#include "GB.h"

GrB_Info GrB_Matrix_dup     // make an exact copy of a matrix
(
    GrB_Matrix *C,          // handle of output matrix to create
    const GrB_Matrix A      // input matrix to copy
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Matrix_dup (&C, A)") ;
    RETURN_IF_NULL (C) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    //--------------------------------------------------------------------------
    // duplicate the matrix
    //--------------------------------------------------------------------------

    return (GB_Matrix_dup (C, A)) ;
}


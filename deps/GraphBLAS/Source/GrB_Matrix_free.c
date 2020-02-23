//------------------------------------------------------------------------------
// GrB_Matrix_free: free a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// free all the content of a matrix.  After GrB_Matrix_free (&A), A is set
// to NULL

#include "GB.h"

GrB_Info GrB_Matrix_free        // free a matrix
(
    GrB_Matrix *A               // handle of matrix to free
)
{ 

    GB_MATRIX_FREE (A) ;
    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GxB_Matrix_resize: change the size of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Matrix_resize      // change the size of a matrix
(
    GrB_Matrix A,               // matrix to modify
    const GrB_Index nrows_new,  // new number of rows in matrix
    const GrB_Index ncols_new   // new number of columns in matrix
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Matrix_resize (A, nrows_new, ncols_new)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    //--------------------------------------------------------------------------
    // resize the matrix
    //--------------------------------------------------------------------------

    return (GB_resize (A, nrows_new, ncols_new)) ;
}


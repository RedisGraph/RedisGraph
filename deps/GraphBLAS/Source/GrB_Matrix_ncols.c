//------------------------------------------------------------------------------
// GrB_Matrix_ncols: number of columns of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Matrix_ncols   // get the number of columns of a matrix
(
    GrB_Index *ncols,       // matrix has ncols columns
    const GrB_Matrix A      // matrix to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Matrix_ncols (&ncols, A)") ;
    RETURN_IF_NULL (ncols) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    // zombies and pending tuples have no effect on nrows
    // but don't bother asserting that fact here

    //--------------------------------------------------------------------------
    // return the number of columns
    //--------------------------------------------------------------------------

    (*ncols) = A->ncols ;
    return (REPORT_SUCCESS) ;
}


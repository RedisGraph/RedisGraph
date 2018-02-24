//------------------------------------------------------------------------------
// GrB_Matrix_nrows: number of rows of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Matrix_nrows   // get the number of rows of a matrix
(
    GrB_Index *nrows,       // matrix has nrows rows
    const GrB_Matrix A      // matrix to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Matrix_nrows (&nrows, A)") ;
    RETURN_IF_NULL (nrows) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    //--------------------------------------------------------------------------
    // get the number of rows
    //--------------------------------------------------------------------------

    return (GB_Matrix_nrows (nrows, A)) ;
}


//------------------------------------------------------------------------------
// GrB_Matrix_nrows: number of rows of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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

    GB_WHERE ("GrB_Matrix_nrows (&nrows, A)") ;
    GB_RETURN_IF_NULL (nrows) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    //--------------------------------------------------------------------------
    // get the number of rows
    //--------------------------------------------------------------------------

    (*nrows) = GB_NROWS (A) ;
    return (GrB_SUCCESS) ;
}


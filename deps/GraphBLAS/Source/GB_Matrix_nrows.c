//------------------------------------------------------------------------------
// GB_Matrix_nrows: number of rows of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Matrix_nrows    // get the number of rows of a matrix
(
    GrB_Index *nrows,       // matrix has nrows rows
    const GrB_Matrix A      // matrix to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (nrows != NULL) ;
    ASSERT (A != NULL) ;

    // zombies and pending tuples have no effect on nrows
    // but don't bother asserting that fact here

    //--------------------------------------------------------------------------
    // return the number of rows
    //--------------------------------------------------------------------------

    (*nrows) = A->nrows ;
    return (REPORT_SUCCESS) ;
}


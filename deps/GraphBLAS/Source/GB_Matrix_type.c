//------------------------------------------------------------------------------
// GB_Matrix_type: return the type of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Matrix_type     // get the type of a matrix
(
    GrB_Type *type,         // returns the type of the matrix
    const GrB_Matrix A      // matrix to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    RETURN_IF_NULL (type) ;
    ASSERT_OK (GB_check (A, "A for type", 0)) ;

    //--------------------------------------------------------------------------
    // return the type
    //--------------------------------------------------------------------------

    (*type) = A->type ;
    return (REPORT_SUCCESS) ;
}


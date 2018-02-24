//------------------------------------------------------------------------------
// GrB_Vector_clear: clears the content of a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Vector_clear   // clear a vector of all entries;
(                           // type and dimension remain unchanged
    GrB_Vector v            // vector to clear
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Vector_clear (v)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (v) ;

    //--------------------------------------------------------------------------
    // clear the vector
    //--------------------------------------------------------------------------

    GB_Matrix_clear ((GrB_Matrix) v) ;

    return (REPORT_SUCCESS) ;
}


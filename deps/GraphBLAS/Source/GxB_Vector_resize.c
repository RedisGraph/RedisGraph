//------------------------------------------------------------------------------
// GxB_Vector_resize: change the size of a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Vector_resize      // change the size of a vector
(
    GrB_Vector u,               // vector to modify
    const GrB_Index nrows_new   // new number of rows in vector
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Vector_resize (u, nrows_new)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ;

    //--------------------------------------------------------------------------
    // resize the vector
    //--------------------------------------------------------------------------

    return (GB_resize ((GrB_Matrix) u, nrows_new, 1)) ;
}


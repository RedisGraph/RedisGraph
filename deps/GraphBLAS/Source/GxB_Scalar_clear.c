//------------------------------------------------------------------------------
// GxB_Scalar_clear: clears the content of a GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Scalar_clear   // clear a GxB_Scalar of its entry
(                           // type and dimension remain unchanged
    GxB_Scalar s            // GxB_Scalar to clear
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Scalar_clear (s)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (s) ;
    ASSERT (GB_SCALAR_OK (s)) ;

    //--------------------------------------------------------------------------
    // clear the GxB_Scalar
    //--------------------------------------------------------------------------

    return (GB_clear ((GrB_Matrix) s, Context)) ;
}


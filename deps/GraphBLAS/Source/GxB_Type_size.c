//------------------------------------------------------------------------------
// GxB_Type_size: return the size of a type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Type_size          // determine the size of the type
(
    size_t *size,               // the sizeof the type
    GrB_Type type               // type to determine the sizeof
)
{
    
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Type_size (&size, type)") ;
    RETURN_IF_NULL (size) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (type) ;

    //--------------------------------------------------------------------------
    // return the size
    //--------------------------------------------------------------------------

    (*size) = type->size ;
    return (REPORT_SUCCESS) ;
}


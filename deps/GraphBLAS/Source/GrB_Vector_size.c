//------------------------------------------------------------------------------
// GrB_Vector_size: dimension of a sparse vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Vector_size    // get the dimension of a vector
(
    GrB_Index *n,           // dimension is n-by-1
    const GrB_Vector v      // vector to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Vector_size (&n, v)") ;
    GB_RETURN_IF_NULL (n) ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;
    ASSERT (GB_VECTOR_OK (v)) ;

    //--------------------------------------------------------------------------
    // get the size
    //--------------------------------------------------------------------------

    (*n) = v->vlen ;
    return (GrB_SUCCESS) ;
}


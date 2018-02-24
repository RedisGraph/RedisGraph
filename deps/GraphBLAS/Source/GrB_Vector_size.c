//------------------------------------------------------------------------------
// GrB_Vector_size: dimension of a sparse vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
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

    WHERE ("GrB_Vector_size (&n, v)") ;
    RETURN_IF_NULL (n) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (v) ;

    //--------------------------------------------------------------------------
    // get the size
    //--------------------------------------------------------------------------

    return (GB_Matrix_nrows (n, (GrB_Matrix) v)) ;
}


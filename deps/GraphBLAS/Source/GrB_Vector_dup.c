//------------------------------------------------------------------------------
// GrB_Vector_dup: make a deep copy of a sparse vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w = u, making a deep copy

#include "GB.h"

GrB_Info GrB_Vector_dup     // make an exact copy of a vector
(
    GrB_Vector *w,          // handle of output vector to create
    const GrB_Vector u      // input vector to copy
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Vector_dup (&w, u)") ;
    RETURN_IF_NULL (w) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ;

    //--------------------------------------------------------------------------
    // duplicate the vector
    //--------------------------------------------------------------------------

    return (GB_Matrix_dup ((GrB_Matrix *) w, (GrB_Matrix) u)) ;
}


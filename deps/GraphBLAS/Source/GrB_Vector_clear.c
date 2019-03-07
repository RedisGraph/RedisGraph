//------------------------------------------------------------------------------
// GrB_Vector_clear: clears the content of a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// parallel: not here but in GB_clear.

#include "GB.h"

GrB_Info GrB_Vector_clear   // clear a vector of all entries;
(                           // type and dimension remain unchanged
    GrB_Vector v            // vector to clear
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Vector_clear (v)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;
    Context->nthreads = GxB_DEFAULT ;   // no descriptor, so use default rule
    ASSERT (GB_VECTOR_OK (v)) ;

    //--------------------------------------------------------------------------
    // clear the vector
    //--------------------------------------------------------------------------

    return (GB_clear ((GrB_Matrix) v, Context)) ;
}


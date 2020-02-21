//------------------------------------------------------------------------------
// GrB_Vector_nvals: number of entries in a sparse vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Vector_nvals   // get the number of entries in a vector
(
    GrB_Index *nvals,       // number of entries
    const GrB_Vector v      // vector to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Vector_nvals (&nvals, v)") ;
    GB_BURBLE_START ("GrB_Vector_nvals") ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;
    ASSERT (GB_VECTOR_OK (v)) ;

    // do not check if nvals is NULL; pending updates must be applied first, in
    // GB_nvals, per Table 2.4 in the spec

    //--------------------------------------------------------------------------
    // get the number of entries
    //--------------------------------------------------------------------------

    GrB_Info info = GB_nvals (nvals, (GrB_Matrix) v, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}


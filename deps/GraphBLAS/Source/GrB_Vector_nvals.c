//------------------------------------------------------------------------------
// GrB_Vector_nvals: number of nonzeros in a sparse vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
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

    WHERE ("GrB_Vector_nvals (&nvals, v)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (v) ;

    // do not check nvals; pending updates must be applied first, in
    // GB_Matrix_nvals, per Table 2.4 in the spec

    //--------------------------------------------------------------------------
    // get the number of entries
    //--------------------------------------------------------------------------

    return (GB_Matrix_nvals (nvals, (GrB_Matrix) v)) ;
}


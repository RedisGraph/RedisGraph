//------------------------------------------------------------------------------
// GrB_Matrix_nvals: number of entries in a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Matrix_nvals   // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const GrB_Matrix A      // matrix to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Matrix_nvals (&nvals, A)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    // do not check nvals; pending updates must be applied first, in
    // GB_nvals, per Table 2.4 in the spec

    //--------------------------------------------------------------------------
    // get the number of entries
    //--------------------------------------------------------------------------

    return (GB_nvals (nvals, A, Context)) ;
}


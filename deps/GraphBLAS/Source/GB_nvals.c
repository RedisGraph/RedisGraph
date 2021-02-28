//------------------------------------------------------------------------------
// GB_nvals: number of entries in a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_nvals           // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const GrB_Matrix A,     // matrix to query
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // delete any lingering zombies and assemble any pending tuples
    // do this as early as possible (see Table 2.4 in spec)
    GB_WAIT (A) ;

    // only now check nvals, for both GrB_Matrix_nvals and GrB_Vector_nvals
    GB_RETURN_IF_NULL (nvals) ;

    //--------------------------------------------------------------------------
    // return the number of entries in the matrix
    //--------------------------------------------------------------------------

    (*nvals) = GB_NNZ (A) ;
    return (GrB_SUCCESS) ;
}


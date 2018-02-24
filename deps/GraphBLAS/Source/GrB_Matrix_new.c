//------------------------------------------------------------------------------
// GrB_Matrix_new: create a new matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The new matrix is nrows-by-ncols, with no entries in it.
// A->p is size ncols+1 and all zero.  Contents A->x and A->i are NULL.
// If this method fails, *A is set to NULL.

#include "GB.h"

GrB_Info GrB_Matrix_new     // create a new matrix with no entries
(
    GrB_Matrix *A,          // handle of matrix to create
    const GrB_Type type,    // type of matrix to create
    const GrB_Index nrows,  // matrix dimension is nrows-by-ncols
    const GrB_Index ncols
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Matrix_new (&A, type, nrows, ncols)") ;
    RETURN_IF_NULL (A) ;
    (*A) = NULL ;
    RETURN_IF_NULL_OR_UNINITIALIZED (type) ;

    if (nrows > GB_INDEX_MAX)
    {
        // problem too large
        return (ERROR (GrB_INVALID_VALUE, (LOG,
            "problem too large: nrows "GBu" > "GBu, nrows, GB_INDEX_MAX))) ;
    }

    if (ncols > GB_INDEX_MAX)
    {
        // problem too large
        return (ERROR (GrB_INVALID_VALUE, (LOG,
            "problem too large: ncols "GBu" > "GBu, ncols, GB_INDEX_MAX))) ;
    }

    //--------------------------------------------------------------------------
    // create the matrix
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_NEW (A, type, nrows, ncols, true, false) ;
    return (info) ;
}


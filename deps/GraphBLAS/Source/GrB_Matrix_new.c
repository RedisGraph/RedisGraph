//------------------------------------------------------------------------------
// GrB_Matrix_new: create a new matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The new matrix is nrows-by-ncols, with no entries in it.  Default format for
// an empty matrix is hypersparse CSC: A->p is size 2 and all zero, A->h is
// size 1, A->plen is 1, and contents A->x and A->i are NULL.  If this method
// fails, *A is set to NULL.

#include "GB.h"

GrB_Info GrB_Matrix_new     // create a new matrix with no entries
(
    GrB_Matrix *A,          // handle of matrix to create
    GrB_Type type,          // type of matrix to create
    GrB_Index nrows,        // matrix dimension is nrows-by-ncols
    GrB_Index ncols
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Matrix_new (&A, type, nrows, ncols)") ;
    GB_RETURN_IF_NULL (A) ;
    (*A) = NULL ;
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;

    if (nrows > GB_INDEX_MAX)
    { 
        // problem too large
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "problem too large: nrows "GBu" exceeds "GBu,
            nrows, GB_INDEX_MAX))) ;
    }

    if (ncols > GB_INDEX_MAX)
    { 
        // problem too large
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "problem too large: ncols "GBu" exceeds "GBu,
            ncols, GB_INDEX_MAX))) ;
    }

    //--------------------------------------------------------------------------
    // create the matrix
    //--------------------------------------------------------------------------

    GrB_Info info ;
    int64_t vlen, vdim ;

    // A is created with auto hypersparsity (typically hypersparse unless
    // vdim <= 1 or hyper_ratio < 0) and default CSR/CSC format.

    bool A_is_csc = GB_Global_is_csc_get ( ) ;

    if (A_is_csc)
    { 
        vlen = (int64_t) nrows ;
        vdim = (int64_t) ncols ;
    }
    else
    { 
        vlen = (int64_t) ncols ;
        vdim = (int64_t) nrows ;
    }

    // *A == NULL ;                 // allocate a new header for A
    GB_NEW (A, type, vlen, vdim, GB_Ap_calloc, A_is_csc,
        GB_AUTO_HYPER, GB_HYPER_DEFAULT, 1, Context) ;
    return (info) ;
}


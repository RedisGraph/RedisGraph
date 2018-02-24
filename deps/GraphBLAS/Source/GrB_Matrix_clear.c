//------------------------------------------------------------------------------
// GrB_Matrix_clear: clears the content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The A->x and A->i content is freed and the column pointers A->p are set to
// zero.  This puts the matrix A in the same state it had after GrB_Matrix_new
// (&A, ...).  The dimensions and type of A are not changed.  To change the
// size or type requires a GrB_Matrix_free (&A) followed by another call to
// GrB_Matrix_new (&A, ...) with the new dimensions and type.

#include "GB.h"

GrB_Info GrB_Matrix_clear   // clear a matrix of all entries;
(                           // type and dimensions remain unchanged
    GrB_Matrix A            // matrix to clear
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Matrix_clear (A)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    //--------------------------------------------------------------------------
    // clear the matrix
    //--------------------------------------------------------------------------

    GB_Matrix_clear (A) ;
    return (REPORT_SUCCESS) ;
}


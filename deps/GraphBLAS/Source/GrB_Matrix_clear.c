//------------------------------------------------------------------------------
// GrB_Matrix_clear: clears the content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The A->x and A->i content is freed and the vector pointers A->p are set to
// zero.  This puts the matrix A in the same state it had after GrB_Matrix_new
// (&A, ...), except that an existing A->Sauna is kept.  The dimensions and
// type of A are not changed.

#include "GB.h"

GrB_Info GrB_Matrix_clear   // clear a matrix of all entries;
(                           // type and dimensions remain unchanged
    GrB_Matrix A            // matrix to clear
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Matrix_clear (A)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    //--------------------------------------------------------------------------
    // clear the matrix (but keep the A->Sauna)
    //--------------------------------------------------------------------------

    return (GB_clear (A, Context)) ;
}


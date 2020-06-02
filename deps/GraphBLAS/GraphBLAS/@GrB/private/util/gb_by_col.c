//------------------------------------------------------------------------------
// gb_by_col: ensure a matrix is stored by column
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The return value A is set to either the input matrix A_input, or the A_copy
// matrix.

#include "gb_matlab.h"

GrB_Matrix gb_by_col            // return the matrix by column
(
    GrB_Matrix *A_copy_handle,  // copy made of A, stored by column, or NULL
    GrB_Matrix A_input          // input matrix, by row or column
)
{

    // get the format of A_input
    GxB_Format_Value fmt ;
    OK (GxB_Matrix_Option_get (A_input, GxB_FORMAT, &fmt)) ;

    GrB_Matrix A_copy = NULL, A ;

    if (fmt == GxB_BY_ROW)
    { 
        // make a deep copy of A_input and change it to be stored by column
        OK (GrB_Matrix_dup (&A_copy, A_input)) ;
        OK (GxB_Matrix_Option_set (A_copy, GxB_FORMAT, GxB_BY_COL)) ;
        A = A_copy ;
    }
    else
    { 
        // A is just A_input, with no change
        A = A_input ;
    }

    // make sure A is finalized
    GrB_Index anz ;
    OK (GrB_Matrix_nvals (&anz, A)) ;

    // return results
    (*A_copy_handle) = A_copy ;
    return (A) ;
}


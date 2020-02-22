//------------------------------------------------------------------------------
// gb_is_vector: determine if a GrB_matrix is a row or column vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

bool gb_is_vector               // true if A is a row or column vector
(
    GrB_Matrix A                // GrB_Matrix to query
)
{

    // check if A is m-by-1 or 1-by-n
    GrB_Index nrows = 0, ncols = 0 ;
    if (A != NULL)
    { 
        OK (GrB_Matrix_nrows (&nrows, A)) ;
        OK (GrB_Matrix_ncols (&ncols, A)) ;
    }
    return (nrows == 1 || ncols == 1) ;
}


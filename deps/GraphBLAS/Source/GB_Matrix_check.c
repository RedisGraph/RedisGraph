//------------------------------------------------------------------------------
// GB_Matrix_check: print a GraphBLAS matrix and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Matrix_check    // check a GraphBLAS matrix
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix
    const GB_diagnostic pr  // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
)
{
    return (GB_object_check (A, name, pr, "matrix")) ;
}


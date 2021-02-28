//------------------------------------------------------------------------------
// GB_mx_Matrix_to_mxArray
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Convert a GraphBLAS sparse matrix to a MATLAB struct containing a
// sparse matrix and a string.  The GraphBLAS matrix is destroyed.

#include "GB_mex.h"

mxArray *GB_mx_Matrix_to_mxArray   // returns the MATLAB mxArray
(
    GrB_Matrix *handle,             // handle of GraphBLAS matrix to convert
    const char *name,
    const bool create_struct        // if true, then return a struct
)
{
    return (GB_mx_object_to_mxArray (handle, name, create_struct)) ;
}


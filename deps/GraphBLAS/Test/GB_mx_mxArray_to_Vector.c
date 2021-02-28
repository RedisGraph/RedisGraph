//------------------------------------------------------------------------------
// GB_mx_mxArray_to_Vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

GrB_Vector GB_mx_mxArray_to_Vector     // returns GraphBLAS version of V
(
    const mxArray *V_matlab,            // MATLAB version of V
    const char *name,                   // name of the argument
    const bool deep_copy,               // if true, return a deep copy
    const bool empty    // if false, 0-by-0 matrices are returned as NULL.
                        // if true, a 0-by-0 matrix is returned.
)
{

    GrB_Matrix V = GB_mx_mxArray_to_Matrix (V_matlab, name, deep_copy, empty) ;
    if (V != NULL && !GB_VECTOR_OK (V))
    {
        mexWarnMsgIdAndTxt ("GB:warn", "must be a column vector") ;
        GB_MATRIX_FREE (&V) ;
        return (NULL) ;
    }
    return ((GrB_Vector) V) ;
}


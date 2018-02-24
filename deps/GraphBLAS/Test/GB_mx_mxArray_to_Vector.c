//------------------------------------------------------------------------------
// GB_mx_mxArray_to_Vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

GrB_Vector GB_mx_mxArray_to_Vector     // returns GraphBLAS version of V
(
    const mxArray *V_matlab,            // MATLAB version of V
    const char *name,                   // name of the argument
    const bool deep_copy                // if true, return a deep copy
)
{

    GrB_Matrix V = GB_mx_mxArray_to_Matrix (V_matlab, name, deep_copy) ;
    if (V != NULL && V->ncols != 1)
    {
        mexWarnMsgIdAndTxt ("GB:warn", "must be a column vector") ;
        GB_MATRIX_FREE (&V) ;
        return (NULL) ;
    }
    return ((GrB_Vector) V) ;
}


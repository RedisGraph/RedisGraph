//------------------------------------------------------------------------------
// gb_mxarray_is_scalar: check if MATLAB mxArray is a non-sparse numeric scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "gb_matlab.h"

bool gb_mxarray_is_scalar   // true if MATLAB array is a scalar
(
    const mxArray *S
)
{ 

    return (S != NULL && mxIsScalar (S) && mxIsNumeric (S) && !mxIsSparse (S)) ;
}


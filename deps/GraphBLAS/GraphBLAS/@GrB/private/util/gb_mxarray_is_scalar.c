//------------------------------------------------------------------------------
// gb_mxarray_is_scalar: check if built-in mxArray is non-sparse numeric scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

#include "gb_interface.h"

bool gb_mxarray_is_scalar   // true if built-in array is a scalar
(
    const mxArray *S
)
{ 

    return (S != NULL && mxIsScalar (S) && mxIsNumeric (S) && !mxIsSparse (S)) ;
}


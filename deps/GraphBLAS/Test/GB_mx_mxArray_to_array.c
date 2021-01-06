//------------------------------------------------------------------------------
// GB_mx_mxArray_to_array: get a dense numerical MATLAB array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_mxArray_to_array    // convert mxArray to array
(
    const mxArray *Xmatlab,     // input MATLAB array
    // output:
    GB_void **X,                // pointer to numerical values (shallow)
    int64_t *nrows,             // number of rows of X
    int64_t *ncols,             // number of columns of X
    GrB_Type *xtype             // GraphBLAS type of X, NULL if error
)
{

    if (!(mxIsNumeric (Xmatlab) || mxIsLogical (Xmatlab)))
    {
        mexWarnMsgIdAndTxt ("GB:warn","input must be numeric or logical array");
    }
    if (mxIsSparse (Xmatlab))
    {
        mexWarnMsgIdAndTxt ("GB:warn","input cannot be sparse") ;
    }

    (*X) = mxGetData (Xmatlab) ;
    (*nrows) = mxGetM (Xmatlab) ;
    (*ncols) = mxGetN (Xmatlab) ;
    (*xtype) = GB_mx_Type (Xmatlab) ;
}


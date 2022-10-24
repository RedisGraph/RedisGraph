//------------------------------------------------------------------------------
// gbdeserialize: deserialize a blob into a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

// gbdeserialize is an interface to GrB_Matrix_deserialize.

// Usage:

// A = gbdeserialize (blob)

#include "gb_interface.h"

#define USAGE "usage: A = GrB.deserialize (blob)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage ((nargin >= 1 || nargin <= 3) && nargout <= 1, USAGE) ;
    CHECK_ERROR (mxGetClassID (pargin [0]) != mxUINT8_CLASS
        || mxGetN (pargin [0]) != 1, "blob must be a uint8 column vector") ;

    //--------------------------------------------------------------------------
    // get the blob
    //--------------------------------------------------------------------------

    void *blob = mxGetData (pargin [0]) ;
    GrB_Index blob_size = (GrB_Index) mxGetM (pargin [0]) ;

    //--------------------------------------------------------------------------
    // deserialize the blob into a matrix
    //--------------------------------------------------------------------------

    GrB_Matrix C = NULL ;
    OK (GrB_Matrix_deserialize (&C, NULL, blob, blob_size)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, KIND_GRB) ;
    GB_WRAPUP ;
}


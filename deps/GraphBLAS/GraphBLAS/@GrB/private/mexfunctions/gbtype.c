//------------------------------------------------------------------------------
// gbtype: type of a GraphBLAS matrix struct, or any MATLAB variable
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input may be any MATLAB variable.  If it is a GraphBLAS G.opaque struct,
// then its internal type is returned.

// Usage

// type = gbtype (X)

#include "gb_matlab.h"

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

    gb_usage (nargin == 1 && nargout <= 1, "usage: type = gbtype (X)") ;

    //--------------------------------------------------------------------------
    // get the type of the matrix
    //--------------------------------------------------------------------------

    mxArray *c = NULL ;
    mxClassID class = mxGetClassID (pargin [0]) ;
    bool is_complex = mxIsComplex (pargin [0]) ;

    if (class == mxSTRUCT_CLASS)
    {
        mxArray *mx_type = mxGetField (pargin [0], 0, "GraphBLASv4") ;
        if (mx_type != NULL)
        { 
            // X is a GraphBLAS G.opaque struct; get its type
            c = mxDuplicateArray (mx_type) ;
        }
    }

    if (c == NULL)
    { 
        // if c is still NULL, then it is not a GraphBLAS opaque struct.
        // get the type of a MATLAB matrix
        c = gb_mxclass_to_mxstring (class, is_complex) ;
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    pargout [0] = c ;
    GB_WRAPUP ;
}


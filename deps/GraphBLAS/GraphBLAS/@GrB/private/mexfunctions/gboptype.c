//------------------------------------------------------------------------------
// gboptype : determine the type of a binary operator from the input types
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

// Usage:

// optype = gboptype (atype, btype)

#include "gb_interface.h"

#define USAGE "usage: c = GrB.optype (atype, btype)"

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

    gb_usage (nargin == 2 && nargout <= 1, USAGE) ;

    //--------------------------------------------------------------------------
    // get atype and btype
    //--------------------------------------------------------------------------

    GrB_Type atype = gb_mxstring_to_type (pargin [0]) ;
    GrB_Type btype = gb_mxstring_to_type (pargin [1]) ;

    //--------------------------------------------------------------------------
    // determine the optype
    //--------------------------------------------------------------------------

    GrB_Type optype = gb_default_type (atype, btype) ;
    CHECK_ERROR (optype == NULL, "unknown type") ;

    //--------------------------------------------------------------------------
    // return result as a string
    //--------------------------------------------------------------------------

    pargout [0] = gb_type_to_mxstring (optype) ;
    GB_WRAPUP ;
}


//------------------------------------------------------------------------------
// gbselectopinfo : print a GraphBLAS selectop (for illustration only)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

// Usage:

// gbselectopinfo (selectop)

#include "gb_interface.h"

#define USAGE "usage: GrB.selectopinfo (selectop) or GrB.selectopinfo (op,type)"

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

    gb_usage (nargin >= 1 && nargin <= 2 && nargout == 0, USAGE) ;

    //--------------------------------------------------------------------------
    // construct the GraphBLAS selectop and print it
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, pargin [0], "select operator") ;

    GrB_Type type = NULL ;
    if (nargin > 1)
    { 
        type = gb_mxstring_to_type (pargin [1]) ;
        CHECK_ERROR (type == NULL, "unknown type") ;
    }

    GxB_SelectOp selop = NULL ;
    GrB_IndexUnaryOp idxunop = NULL ;
    bool ignore1, ignore2 ;
    int64_t ignore3 = 0 ;

    gb_mxstring_to_selectop (&idxunop, &selop, &ignore1, &ignore2, &ignore3,
        pargin [0], type) ;

    if (selop != NULL)
    { 
        OK (GxB_SelectOp_fprint (selop, opstring, GxB_COMPLETE, NULL)) ;
    }
    else
    { 
        OK (GxB_IndexUnaryOp_fprint (idxunop, opstring, GxB_COMPLETE, NULL)) ;
    }

    GB_WRAPUP ;
}


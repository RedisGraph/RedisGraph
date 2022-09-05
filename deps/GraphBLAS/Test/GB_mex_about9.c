//------------------------------------------------------------------------------
// GB_mex_about9: still more basic tests (not for Windows)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Windows is limited to user-defined types of size 128 or less.

#include "GB_mex.h"
#include "GB_mex_errors.h"

#define USAGE "GB_mex_about9"
#define FREE_ALL ;
#define GET_DEEP_COPY ;
#define FREE_DEEP_COPY ;

typedef struct
{
    double stuff [32] ;
}
bigtype ;

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // startup GraphBLAS
    //--------------------------------------------------------------------------

    bool malloc_debug = GB_mx_get_global (true) ;

    //--------------------------------------------------------------------------
    // user-defined type of 256 bytes
    //--------------------------------------------------------------------------

    GrB_Type BigType ;
    OK (GxB_Type_new (&BigType, sizeof (bigtype), "bigtype",
        "typedef struct { double stuff [32] ; } bigtype")) ;
    OK (GxB_Type_fprint (BigType, "(256-byte big type)", GxB_COMPLETE,
        stdout)) ;
    OK (GrB_Type_free (&BigType)) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    GB_mx_put_global (true) ;
    printf ("\nGB_mex_about9: all tests passed\n\n") ;
}


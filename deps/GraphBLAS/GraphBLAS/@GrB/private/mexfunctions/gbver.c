//------------------------------------------------------------------------------
// gbver: struct with SuiteSparse:GraphBLAS version
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// v = gbver

#include "gb_matlab.h"

static const char *vfields [3] = { "Name", "Version", "Date" } ;

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

    gb_usage (nargin == 0 && nargout <= 1, "usage: v = gbver") ;

    //--------------------------------------------------------------------------
    // get the version and date information and return it as a MATLAB struct
    //--------------------------------------------------------------------------

    int version [3] ;
    OK (GxB_Global_Option_get (GxB_LIBRARY_VERSION, version)) ;

    char *date ;
    OK (GxB_Global_Option_get (GxB_LIBRARY_DATE, &date)) ;

    if (nargout == 0)
    {
        char *license, *about, *spec, *url ;
        printf ("----------------------------------------"
                "-----------------------------------\n") ;
        OK (GxB_Global_Option_get (GxB_LIBRARY_ABOUT, &about)) ;
        printf ("%s\n", about) ;
        printf ("Version: %d.%d.%d (%s)\n\n",
                version [0], version [1], version [2], date) ;
        OK (GxB_Global_Option_get (GxB_LIBRARY_LICENSE, &license)) ;
        printf ("License:\n%s\n", license) ;
        OK (GxB_Global_Option_get (GxB_API_ABOUT, &spec)) ;
        printf ("Spec:\n%s\n", spec) ;
        OK (GxB_Global_Option_get (GxB_API_URL, &url)) ;
        printf ("URL: %s\n", url) ;
        printf ("----------------------------------------"
                "-----------------------------------\n") ;
    }
    else
    {
        #define LEN 256
        char s [LEN+1] ;
        snprintf (s, LEN, "%d.%d.%d", version [0], version [1], version [2]) ;
        pargout [0] = mxCreateStructMatrix (1, 1, 3, vfields) ;
        mxSetFieldByNumber (pargout [0], 0, 0,
                mxCreateString ("SuiteSparse:GraphBLAS")) ;
        mxSetFieldByNumber (pargout [0], 0, 1, mxCreateString (s)) ;
        mxSetFieldByNumber (pargout [0], 0, 2, mxCreateString (date)) ;
    }

    GB_WRAPUP ;
}


//------------------------------------------------------------------------------
// GB_mex_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns the status of all global settings.

#include "GB_mex.h"

#define USAGE "[nthreads_max threading thread_safety format hyperratio" \
"name version date about license compiledate compiletime api api_about" \
" chunk] = GB_mex_init"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    GxB_init (GrB_NONBLOCKING, mxMalloc, mxCalloc, mxRealloc, mxFree, false) ;
    GB_WHERE (USAGE) ;
    GB_Global_abort_function_set (GB_mx_abort) ;
    GB_Global_malloc_tracking_set (true) ;

    // MATLAB default is by column
    GxB_set (GxB_FORMAT, GxB_BY_COL) ;

    int nthreads_max ;
    GxB_get (GxB_NTHREADS, &nthreads_max) ;
    pargout [0] = mxCreateDoubleScalar (nthreads_max) ;

    GxB_Thread_Model threading ;
    GxB_get (GxB_THREADING, &threading) ;
    pargout [1] = mxCreateDoubleScalar (threading) ;

    GxB_Thread_Model thread_safety ;
    GxB_get (GxB_THREAD_SAFETY, &thread_safety) ;
    pargout [2] = mxCreateDoubleScalar (thread_safety) ;

    GxB_Format_Value format ;
    GxB_get (GxB_FORMAT, &format) ;
    pargout [3] = mxCreateDoubleScalar (format) ;

    double hyperratio ;
    GxB_get (GxB_HYPER, &hyperratio) ;
    pargout [4] = mxCreateDoubleScalar (hyperratio) ;

    char *name ;
    GxB_get (GxB_LIBRARY_NAME, &name) ;
    pargout [5] = mxCreateString (name) ;

    int version [3] ;
    GxB_get (GxB_LIBRARY_VERSION, version) ;
    pargout [6] = mxCreateDoubleMatrix (1, 3, mxREAL) ;
    double *p = mxGetPr (pargout [6]) ;
    p [0] = version [0] ;
    p [1] = version [1] ;
    p [2] = version [2] ;

    char *date ;
    GxB_get (GxB_LIBRARY_DATE, &date) ;
    pargout [7] = mxCreateString (date) ;

    char *about ;
    GxB_get (GxB_LIBRARY_ABOUT, &about) ;
    pargout [8] = mxCreateString (about) ;

    char *license ;
    GxB_get (GxB_LIBRARY_LICENSE, &license) ;
    pargout [9] = mxCreateString (license) ;

    char *compile_date ;
    GxB_get (GxB_LIBRARY_COMPILE_DATE, &compile_date) ;
    pargout [10] = mxCreateString (compile_date) ;

    char *compile_time ;
    GxB_get (GxB_LIBRARY_COMPILE_TIME, &compile_time) ;
    pargout [11] = mxCreateString (compile_time) ;

    int api [3] ;
    GxB_get (GxB_API_VERSION, api) ;
    pargout [12] = mxCreateDoubleMatrix (1, 3, mxREAL) ;
    double *a = mxGetPr (pargout [12]) ;
    a [0] = api [0] ;
    a [1] = api [1] ;
    a [2] = api [2] ;

    char *api_about ;
    GxB_get (GxB_API_ABOUT, &api_about) ;
    pargout [13] = mxCreateString (api_about) ;

    double chunk ;
    GxB_get (GxB_CHUNK, &chunk) ;
    pargout [14] = mxCreateDoubleScalar (chunk) ;

    GrB_finalize ( ) ;
}


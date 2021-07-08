//------------------------------------------------------------------------------
// GB_mx_get_global: get variables from the global workspace
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_get_global       // true if doing malloc_debug
(
    bool cover              // true if doing statement coverage
)
{

    //--------------------------------------------------------------------------
    // get malloc debug
    //--------------------------------------------------------------------------

    bool malloc_debug = false ;
    bool *debug = NULL ;
    const mxArray *debug_builtin = NULL ;
    debug_builtin = mexGetVariablePtr ("global", "GraphBLAS_debug") ;
    if (debug_builtin == NULL || mxIsEmpty (debug_builtin))
    {
        // doesn't exist; create it and set it to false
        debug_builtin = GB_mx_create_full (1, 1, GrB_BOOL) ;
        debug = (bool *) mxGetData (debug_builtin) ;
        if (debug == NULL) mexErrMsgTxt ("debug_builtin null?!") ;
        debug [0] = false ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_debug", debug_builtin) ;
    }
    else
    {
        debug = (bool *) mxGetData (debug_builtin) ;
        if (debug == NULL) mexErrMsgTxt ("debug_builtin null!") ;
        malloc_debug = debug [0] ;
        // if (malloc_debug) printf ("GraphBLAS malloc debug enabled\n") ;
    }

    //--------------------------------------------------------------------------
    // get test coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOVER
    if (cover) GB_cover_get ( ) ;
    #endif

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    bool burble = GB_Global_burble_get ( ) ;            // save current burble
    GB_Global_GrB_init_called_set (false) ;
//  GxB_init (GrB_NONBLOCKING, mxMalloc, NULL, NULL, mxFree, false) ;
    GxB_init (GrB_NONBLOCKING, mxMalloc, mxCalloc, mxRealloc, mxFree, false) ;
    ASSERT (GB_Global_nmalloc_get ( ) == 0) ;
    GB_Global_abort_function_set (GB_mx_abort) ;
    GB_Global_malloc_tracking_set (true) ;
    GxB_Global_Option_set_(GxB_FORMAT, GxB_BY_COL) ;
    GxB_Global_Option_set_(GxB_BURBLE, burble) ;        // restore the burble
    GxB_Global_Option_set_(GxB_PRINTF, mexPrintf) ;

    //--------------------------------------------------------------------------
    // get nthreads
    //--------------------------------------------------------------------------

    int *nthreads = NULL ;
    const mxArray *nthreads_builtin = NULL ;
    nthreads_builtin = mexGetVariablePtr ("global", "GraphBLAS_nthreads") ;
    if (nthreads_builtin == NULL || mxIsEmpty (nthreads_builtin))
    {
        // doesn't exist; create it and set it to 1
        nthreads_builtin = GB_mx_create_full (1, 1, GrB_INT32) ;
        nthreads = (int32_t *) mxGetData (nthreads_builtin) ;
        if (nthreads == NULL) mexErrMsgTxt ("nthreads_builtin null?!") ;
        nthreads [0] = 1 ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_nthreads", nthreads_builtin) ;
    }
    else
    {
        nthreads = (int32_t *) mxGetData (nthreads_builtin) ;
        if (nthreads == NULL) mexErrMsgTxt ("nthreads_builtin null!") ;
    }

    GxB_Global_Option_set_(GxB_NTHREADS, nthreads [0]) ;

    //--------------------------------------------------------------------------
    // get chunk
    //--------------------------------------------------------------------------

    double *chunk = NULL ;
    const mxArray *chunk_builtin = NULL ;
    chunk_builtin = mexGetVariablePtr ("global", "GraphBLAS_chunk") ;
    if (chunk_builtin == NULL || mxIsEmpty (chunk_builtin))
    {
        // doesn't exist; create it and set it to GB_CHUNK_DEFAULT
        chunk_builtin = GB_mx_create_full (1, 1, GrB_FP64) ;
        chunk = (double *) mxGetData (chunk_builtin) ;
        if (chunk == NULL) mexErrMsgTxt ("chunk_builtin null?!") ;
        chunk [0] = GB_CHUNK_DEFAULT ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_chunk", chunk_builtin) ;
    }
    else
    {
        chunk = (double *) mxGetData (chunk_builtin) ;
        if (chunk == NULL) mexErrMsgTxt ("chunk_builtin null!") ;
    }

    GxB_Global_Option_set_(GxB_CHUNK, chunk [0]) ;

    //--------------------------------------------------------------------------
    // get GraphBLAS_complex flag and allocate the complex type and operators
    //--------------------------------------------------------------------------

    bool *builtin_complex = NULL ;
    const mxArray *builtin_complex_builtin = NULL ;
    builtin_complex_builtin =
        mexGetVariablePtr ("global", "GraphBLAS_builtin_complex") ;
    if (builtin_complex_builtin == NULL || mxIsEmpty (builtin_complex_builtin))
    {
        // doesn't exist; create it and set it to TRUE
        builtin_complex_builtin = GB_mx_create_full (1, 1, GrB_BOOL) ;
        builtin_complex = (bool *) mxGetData (builtin_complex_builtin) ;
        if (builtin_complex == NULL)
        {
            mexErrMsgTxt ("builtin_complex_builtin null?!") ;
        }
        builtin_complex [0] = true ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_builtin_complex",
            builtin_complex_builtin) ;
    }
    else
    {
        builtin_complex = (bool *) mxGetData (builtin_complex_builtin) ;
        if (builtin_complex == NULL)
        {
            mexErrMsgTxt ("builtin_complex_builtin null!") ;
        }
    }

    Complex_init (builtin_complex [0]) ;

    //--------------------------------------------------------------------------
    // return malloc debug status
    //--------------------------------------------------------------------------

    return (malloc_debug) ;
}


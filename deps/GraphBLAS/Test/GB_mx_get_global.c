//------------------------------------------------------------------------------
// GB_mx_get_global: get the GraphBLAS thread-local storage from MATLAB
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Get the variable 'GraphBLAS_debug' from the MATLAB global workspace.
// If it doesn't exist, create it and set it to false.

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
    const mxArray *debug_matlab = NULL ;
    debug_matlab = mexGetVariablePtr ("global", "GraphBLAS_debug") ;
    if (debug_matlab == NULL || mxIsEmpty (debug_matlab))
    {
        // doesn't exist; create it and set it to false
        debug_matlab = mxCreateNumericMatrix (1, 1, mxLOGICAL_CLASS, mxREAL) ;
        debug = (bool *) mxGetData (debug_matlab) ;
        if (debug == NULL) mexErrMsgTxt ("debug_matlab null?!") ;
        debug [0] = false ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_debug", debug_matlab) ;
    }
    else
    {
        debug = (bool *) mxGetData (debug_matlab) ;
        if (debug == NULL) mexErrMsgTxt ("debug_matlab null!") ;
        malloc_debug = debug [0] ;
        // if (malloc_debug) printf ("GraphBLAS malloc debug enabled\n") ;
    }


    //--------------------------------------------------------------------------
    // clear the time
    //--------------------------------------------------------------------------

    GB_mx_clear_time ( ) ;

    //--------------------------------------------------------------------------
    // get test coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOVER
    if (cover) GB_cover_get ( ) ;
    #endif

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    GB_Global_GrB_init_called_set (false) ;
    GxB_init (GrB_NONBLOCKING, mxMalloc, mxCalloc, mxRealloc, mxFree, false) ;
    ASSERT (GB_Global_nmalloc_get ( ) == 0) ;
    GB_Global_abort_function_set (GB_mx_abort) ;
    GB_Global_malloc_tracking_set (true) ;
    GxB_set (GxB_FORMAT, GxB_BY_COL) ;

    //--------------------------------------------------------------------------
    // get nthreads
    //--------------------------------------------------------------------------

    int *nthreads = NULL ;
    const mxArray *nthreads_matlab = NULL ;
    nthreads_matlab = mexGetVariablePtr ("global", "GraphBLAS_nthreads") ;
    if (nthreads_matlab == NULL || mxIsEmpty (nthreads_matlab))
    {
        // doesn't exist; create it and set it to 1
        nthreads_matlab = mxCreateNumericMatrix (1, 1, mxINT32_CLASS, mxREAL) ;
        nthreads = (int32_t *) mxGetData (nthreads_matlab) ;
        if (nthreads == NULL) mexErrMsgTxt ("nthreads_matlab null?!") ;
        nthreads [0] = 1 ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_nthreads", nthreads_matlab) ;
    }
    else
    {
        nthreads = (int32_t *) mxGetData (nthreads_matlab) ;
        if (nthreads == NULL) mexErrMsgTxt ("nthreads_matlab null!") ;
    }

    GxB_set (GxB_NTHREADS, nthreads [0]) ;

    //--------------------------------------------------------------------------
    // get chunk
    //--------------------------------------------------------------------------

    double *chunk = NULL ;
    const mxArray *chunk_matlab = NULL ;
    chunk_matlab = mexGetVariablePtr ("global", "GraphBLAS_chunk") ;
    if (chunk_matlab == NULL || mxIsEmpty (chunk_matlab))
    {
        // doesn't exist; create it and set it to GB_CHUNK_DEFAULT
        chunk_matlab = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL) ;
        chunk = (double *) mxGetData (chunk_matlab) ;
        if (chunk == NULL) mexErrMsgTxt ("chunk_matlab null?!") ;
        chunk [0] = GB_CHUNK_DEFAULT ;
        // copy it into the global workspace
        mexPutVariable ("global", "GraphBLAS_chunk", chunk_matlab) ;
    }
    else
    {
        chunk = (double *) mxGetData (chunk_matlab) ;
        if (chunk == NULL) mexErrMsgTxt ("chunk_matlab null!") ;
    }

    GxB_set (GxB_CHUNK, chunk [0]) ;

    //--------------------------------------------------------------------------
    // allocate the complex type and operators
    //--------------------------------------------------------------------------

    Complex_init ( ) ;

    //--------------------------------------------------------------------------
    // return malloc debug status
    //--------------------------------------------------------------------------

    return (malloc_debug) ;
}


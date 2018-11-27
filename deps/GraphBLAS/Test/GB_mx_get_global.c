//------------------------------------------------------------------------------
// GB_mx_get_global: get the GraphBLAS thread-local storage from MATLAB
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Get the variable 'GraphBLAS_debug' from the MATLAB global workspace.
// If it doesn't exist, create it and set it to false.

#include "GB_mex.h"

bool GB_mx_get_global      // true if doing malloc_debug
(
    bool cover
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

    GB_Global.GrB_init_called = false ;
    GrB_init (GrB_NONBLOCKING) ;
    GxB_set (GxB_FORMAT, GxB_BY_COL) ;
    ASSERT (GB_Global.nmalloc == 0) ;
    Complex_init ( ) ;

    //--------------------------------------------------------------------------
    // return malloc debug status
    //--------------------------------------------------------------------------

    // the caller will set GB_Global.malloc_debug, not done here
    return (malloc_debug) ;
}


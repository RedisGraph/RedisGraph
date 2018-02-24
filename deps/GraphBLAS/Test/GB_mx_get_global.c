//------------------------------------------------------------------------------
// GB_mx_get_global: get the GraphBLAS thread-local storage from MATLAB
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Get the variable 'GraphBLAS_debug' from the MATLAB global workspace.
// If it doesn't exist, create it and set it to false.

#include "GB_mex.h"

bool GB_mx_get_global      // true if doing malloc_debug
(
)
{

    //--------------------------------------------------------------------------
    // get malloc debug
    //--------------------------------------------------------------------------

    GB_thread_local.info = GrB_SUCCESS ;
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
    // get test coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOVER
    gbcover_get ( ) ;
    #endif

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    GrB_init (GrB_NONBLOCKING) ;
    Complex_init ( ) ;

    //--------------------------------------------------------------------------
    // return malloc debug status
    //--------------------------------------------------------------------------

    // the caller will set GB_Global.malloc_debug, not done here
    return (malloc_debug) ;
}


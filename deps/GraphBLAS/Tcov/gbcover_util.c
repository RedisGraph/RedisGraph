//------------------------------------------------------------------------------
// gbcover_util.c: utilities for test coverage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// These functions are compiled along with the GraphBLAS mexFunctions, to
// allow them to copy the statement coverage counts to and from the MATLAB
// global workspace.

#include "GB_mex.h"

//------------------------------------------------------------------------------
// gbcover_get: copy coverage counts from the MATLAB workspace
//------------------------------------------------------------------------------

// This function is called when a GraphBLAS mexFunction starts.
// GraphBLAS_gbcov is an int64 MATLAB array in the global MATLAB workspace.
// It's size is controlled by gbcover_max, defined in gbcover_finish.c.  If the
// array is empty in the workspace, or if it doesn't exist, it is created with
// the correct size.  Then the internal gbcov array is copied into it.

void gbcover_get ( )
{

    // get GraphBLAS_gbcov from MATLAB global workspace
    mxArray *gbcov_matlab = NULL ;
    gbcov_matlab = (mxArray *) mexGetVariablePtr ("global", "GraphBLAS_gbcov") ;

    if (gbcov_matlab == NULL || mxIsEmpty (gbcov_matlab))
    {
        // doesn't exist; create it and set it to zero
        gbcov_matlab = mxCreateNumericMatrix (1, gbcover_max,
            mxINT64_CLASS, mxREAL) ;
        // copy it back to the global MATLAB workspace
        mexPutVariable ("global", "GraphBLAS_gbcov", gbcov_matlab) ;
    }

    // it should exist now, but double-check
    if (gbcov_matlab == NULL || mxIsEmpty (gbcov_matlab))
    {
        mexErrMsgTxt ("gbcov_matlab still null!") ;
    }

    // get a pointer to the content of the GraphBLAS_gbcov array in the
    // MATLAB workspace
    int64_t *g = (int64_t *) mxGetData (gbcov_matlab) ;

    // getting paranoid here; this should never happen
    if (g == NULL) mexErrMsgTxt ("g null!") ;

    // copy the count from the MATLAB GraphBLAS_gbcov into gbcov
    memcpy (gbcov, g, gbcover_max * sizeof (int64_t)) ;
}

//------------------------------------------------------------------------------
// gbcover_put: copy coverage counts back to the MATLAB workspace
//------------------------------------------------------------------------------

// This function is called when a GraphBLAS mexFunction finishes.  It copies
// the updated statement coverage counters in the gbcov array back to the
// GraphBLAS_gbcov array in the MATLAB global workspace where it can be
// analyzed.

void gbcover_put ( )
{

    // create a MATLAB array with the right size
    mxArray * gbcov_matlab = mxCreateNumericMatrix (1, gbcover_max,
            mxINT64_CLASS, mxREAL) ;

    // copy the updated gbcov counter array into the MATLAB array
    int64_t *g = (int64_t *) mxGetData (gbcov_matlab) ;
    memcpy (g, gbcov, gbcover_max * sizeof (int64_t)) ;

    // put the MATLAB array into the global workspace, overwriting the
    // version that was already there
    mexPutVariable ("global", "GraphBLAS_gbcov", gbcov_matlab) ;
}


//------------------------------------------------------------------------------
// gbcov_util.c: utilities for test coverage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// These functions are compiled along with the GraphBLAS mexFunctions, to
// allow them to copy the statement coverage counts to and from the MATLAB
// global workspace.

#include "gb_matlab.h"

//------------------------------------------------------------------------------
// gbcov_get: copy coverage counts from the MATLAB workspace
//------------------------------------------------------------------------------

// This function is called when a GraphBLAS mexFunction starts.
// gbcov_global is an int64 MATLAB array in the global MATLAB workspace.
// Its size is controlled by gbcov_max, defined in gbcovfinish.c.  If the
// array is empty in the workspace, or if it doesn't exist, it is created with
// the correct size.  Then the internal gbcov array is copied into it.

void gbcov_get ( )
{

    // get gbcov_global from MATLAB global workspace
    mxArray *GB_cov_matlab = NULL ;
    GB_cov_matlab = (mxArray *) mexGetVariablePtr ("global", "gbcov_global") ;

    if (GB_cov_matlab == NULL || mxIsEmpty (GB_cov_matlab))
    {
        // doesn't exist; create it and set it to zero
        GB_cov_matlab = mxCreateNumericMatrix (1, gbcov_max,
            mxINT64_CLASS, mxREAL) ;
        // copy it back to the global MATLAB workspace
        mexPutVariable ("global", "gbcov_global", GB_cov_matlab) ;
    }

    // it should exist now, but double-check
    if (GB_cov_matlab == NULL || mxIsEmpty (GB_cov_matlab))
    {
        mexErrMsgTxt ("GB_cov_matlab still null!") ;
    }

    // get a pointer to the content of the gbcov_global array in the
    // MATLAB workspace
    int64_t *g = (int64_t *) mxGetData (GB_cov_matlab) ;

    // getting paranoid here; this should never happen
    if (g == NULL) mexErrMsgTxt ("g null!") ;

    // copy the count from the MATLAB gbcov_global into gbcov
    memcpy (gbcov, g, gbcov_max * sizeof (int64_t)) ;
}

//------------------------------------------------------------------------------
// gbcov_put: copy coverage counts back to the MATLAB workspace
//------------------------------------------------------------------------------

// This function is called when a GraphBLAS mexFunction finishes.  It copies
// the updated statement coverage counters in the gbcov array back to the
// gbcov_global array in the MATLAB global workspace where it can be
// analyzed.

void gbcov_put ( )
{
    // create a MATLAB array with the right size
    mxArray * GB_cov_matlab = mxCreateNumericMatrix (1, gbcov_max,
            mxINT64_CLASS, mxREAL) ;

    // copy the updated gbcov counter array into the MATLAB array
    int64_t *g = (int64_t *) mxGetData (GB_cov_matlab) ;
    memcpy (g, gbcov, gbcov_max * sizeof (int64_t)) ;

    // put the MATLAB array into the global workspace, overwriting the
    // version that was already there
    mexPutVariable ("global", "gbcov_global", GB_cov_matlab) ;
}


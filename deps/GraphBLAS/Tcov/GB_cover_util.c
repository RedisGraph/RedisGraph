//------------------------------------------------------------------------------
// GB_cover_util.c: utilities for test coverage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// These functions are compiled along with the GraphBLAS mexFunctions, to
// allow them to copy the statement coverage counts to and from the
// global workspace.

#include "GB_mex.h"

//------------------------------------------------------------------------------
// GB_cover_get: copy coverage counts from the workspace
//------------------------------------------------------------------------------

// This function is called when a GraphBLAS mexFunction starts.
// GraphBLAS_grbcov is an int64 built-in array in the global workspace.
// Its size is controlled by GB_cover_max, defined in GB_cover_finish.c.  If the
// array is empty in the workspace, or if it doesn't exist, it is created with
// the correct size.  Then the internal GB_cov array is copied into it.

void GB_cover_get ( )
{

    // get GraphBLAS_grbcov from global workspace
    mxArray *GB_cov_mx = NULL ;
    GB_cov_mx = (mxArray *) mexGetVariablePtr ("global", "GraphBLAS_grbcov") ;

    if (GB_cov_mx == NULL || mxIsEmpty (GB_cov_mx))
    {
        // doesn't exist; create it and set it to zero
        GB_cov_mx = mxCreateNumericMatrix (1, GB_cover_max,
            mxINT64_CLASS, mxREAL) ;
        // copy it back to the global workspace
        mexPutVariable ("global", "GraphBLAS_grbcov", GB_cov_mx) ;
    }

    // it should exist now, but double-check
    if (GB_cov_mx == NULL || mxIsEmpty (GB_cov_mx))
    {
        mexErrMsgTxt ("GB_cov_mx still null!") ;
    }

    // get a pointer to the content of the GraphBLAS_grbcov array in the
    // workspace
    int64_t *g = (int64_t *) mxGetData (GB_cov_mx) ;

    // getting paranoid here; this should never happen
    if (g == NULL) mexErrMsgTxt ("g null!") ;
    // if (GB_cov == NULL) mexErrMsgTxt ("GB_cov is NULL!") ;

    // copy the count from the GraphBLAS_grbcov into GB_cov
    memcpy (GB_cov, g, GB_cover_max * sizeof (int64_t)) ;
}

//------------------------------------------------------------------------------
// GB_cover_put: copy coverage counts back to the workspace
//------------------------------------------------------------------------------

// This function is called when a GraphBLAS mexFunction finishes.  It copies
// the updated statement coverage counters in the GB_cov array back to the
// GraphBLAS_grbcov array in the global workspace where it can be
// analyzed.

void GB_cover_put ( )
{
    // printf ("GB_cover_put: %d\n", GB_cover_max) ;

    // create a built-in array with the right size
    mxArray * GB_cov_mx = mxCreateNumericMatrix (1, GB_cover_max,
            mxINT64_CLASS, mxREAL) ;

    // copy the updated GB_cov counter array into the built-in array
    int64_t *g = (int64_t *) mxGetData (GB_cov_mx) ;
    memcpy (g, GB_cov, GB_cover_max * sizeof (int64_t)) ;

    // put the built-in array into the global workspace, overwriting the
    // version that was already there
    mexPutVariable ("global", "GraphBLAS_grbcov", GB_cov_mx) ;
}


//------------------------------------------------------------------------------
// GB_mx_put_time: put the time back to the global MATLAB workspace
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

double grbtime = 0, tic [2] = {0,0} ;

void GB_mx_put_time (void)
{

    // create a MATLAB array with the right size
    mxArray * grbresults_matlab = GB_mx_create_full (1, 2, GrB_FP64) ;

    // copy the time into the MATLAB array
    double *t = (double *) mxGetData (grbresults_matlab) ;

    t [0] = grbtime ;
    t [1] = 0 ;

    grbtime = 0 ;

    // put the MATLAB array into the global workspace, overwriting the
    // version that was already there
    mexPutVariable ("global", "GraphBLAS_results", grbresults_matlab) ;
}


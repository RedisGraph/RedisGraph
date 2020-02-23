//------------------------------------------------------------------------------
// GB_mx_put_time: put the time back to the global MATLAB workspace
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

double grbtime = 0, tic [2] = {0,0} ;

void GB_mx_put_time
(
    GrB_Desc_Value AxB_method_used
)
{

    // create a MATLAB array with the right size
    mxArray * grbresults_matlab = mxCreateNumericMatrix (1, 2,
            mxDOUBLE_CLASS, mxREAL) ;

    // copy the time into the MATLAB array
    double *t = (double *) mxGetData (grbresults_matlab) ;

    t [0] = grbtime ;
    t [1] = AxB_method_used ;

    grbtime = 0 ;

    // put the MATLAB array into the global workspace, overwriting the
    // version that was already there
    mexPutVariable ("global", "GraphBLAS_results", grbresults_matlab) ;
}


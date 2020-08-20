//------------------------------------------------------------------------------
// gb_mxarray_is_empty: check if a MATLAB mxArray is empty
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

bool gb_mxarray_is_empty    // true if MATLAB array is NULL, or 2D and 0-by-0
(
    const mxArray *S
)
{ 

    return ((S == NULL)
        || ((mxGetNumberOfDimensions (S) == 2) &&
            (mxGetM (S) == 0) && (mxGetN (S) == 0))) ;
}


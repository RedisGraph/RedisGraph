//------------------------------------------------------------------------------
// GB_mx_mxArray_to_indices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Get a list of indices from MATLAB

#include "GB_mex.h"

bool GB_mx_mxArray_to_indices      // true if successful, false otherwise
(
    GrB_Index **handle,             // index array returned
    const mxArray *I_matlab,        // MATLAB mxArray to get
    GrB_Index *ni                   // length of the array
)
{

    (*handle) = NULL ;

    GrB_Index *I ;
    if (I_matlab == NULL || mxIsEmpty (I_matlab))
    {
        I = (GrB_Index *) GrB_ALL ;       // like the ":" in C=A(:,j)
        (*ni) = 0 ;
    }
    else
    {
        if (!mxIsClass (I_matlab, "uint64"))
        {
            mexWarnMsgIdAndTxt ("GB:warn","indices must be uint64") ;
            return (false) ;
        }
        I = mxGetData (I_matlab) ;
        (*ni) = (uint64_t) mxGetNumberOfElements (I_matlab) ;
    }

    (*handle) = I ;
    return (true) ;
}


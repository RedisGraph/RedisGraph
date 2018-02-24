//------------------------------------------------------------------------------
// GB_mex_binaryop: parse a binaryop, for testing
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // check inputs
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: GB_mex_binaryop (binaryop_struct))") ;
    }

    GrB_BinaryOp binaryop = NULL ;
    GB_mx_mxArray_to_BinaryOp (&binaryop, pargin [0], "binaryop",
        GB_PLUS_opcode, mxDOUBLE_CLASS, false, false) ;

    GrB_Info info = GB_check (binaryop, "binaryop", 3) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt (GrB_error ( )) ;
    }
}


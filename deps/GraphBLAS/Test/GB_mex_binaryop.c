//------------------------------------------------------------------------------
// GB_mex_binaryop: parse a binaryop, for testing
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "GB_mex_binaryop (binaryop_struct))"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    GrB_BinaryOp binaryop = NULL ;
    GB_mx_mxArray_to_BinaryOp (&binaryop, pargin [0], "binaryop",
        GB_PLUS_opcode, mxDOUBLE_CLASS, false, false) ;

    GrB_Info info = GB_BinaryOp_check (binaryop, "binaryop", GxB_COMPLETE,
        stdout, Context) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt (GrB_error ( )) ;
    }
}


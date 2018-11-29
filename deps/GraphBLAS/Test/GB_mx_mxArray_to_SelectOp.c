//------------------------------------------------------------------------------
// GB_mx_mxArray_to_SelectOp
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Convert a MATLAB string to a built-in GraphBLAS SelectOp.

#include "GB_mex.h"

bool GB_mx_mxArray_to_SelectOp          // true if successful
(
    GxB_SelectOp *handle,               // returns GraphBLAS version of op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name                    // name of the argument
)
{

    (*handle) = NULL ;
    const mxArray *opname_mx = NULL ;

    if (op_matlab == NULL || mxIsEmpty (op_matlab))
    {
        mexWarnMsgIdAndTxt ("GB:warn", "select op missing") ;
        return (false) ;
    }
    else if (mxIsChar (op_matlab))
    {
        // op is a string.  default class will be used
        opname_mx = op_matlab ;
    }
    else
    {
        mexWarnMsgIdAndTxt ("GB:warn", "select op must be string") ;
        return (false) ;
    }

    // find the corresponding built-in GraphBLAS operator
    GxB_SelectOp op ;

    // get the string
    #define LEN 256
    char opname [LEN+2] ;
    int len = GB_mx_mxArray_to_string (opname, LEN, opname_mx) ;
    if (len < 0)
    {
        return (false) ;
    }

    if      (MATCH (opname, "tril"     )) { op = GxB_TRIL ; }
    else if (MATCH (opname, "triu"     )) { op = GxB_TRIU ; }
    else if (MATCH (opname, "diag"     )) { op = GxB_DIAG ; }
    else if (MATCH (opname, "offdiag"  )) { op = GxB_OFFDIAG ; }
    else if (MATCH (opname, "nonzero"  )) { op = GxB_NONZERO ; }
    else
    {
        mexWarnMsgIdAndTxt ("GB:warn", "unknown select op") ;
        return (false) ;
    }

    // return the op
    ASSERT_OK (GB_check (op, name, GB0)) ;
    (*handle) = op ;
    return (true) ;
}


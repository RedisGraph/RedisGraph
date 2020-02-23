//------------------------------------------------------------------------------
// GB_mx_mxArray_to_UnaryOp
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Convert a MATLAB string or struct to a built-in GraphBLAS UnaryOp.  The
// mxArray is either a struct containing two terms: opname (an operator name),
// and an optional MATLAB string opclass (a string, 'logical', 'double', etc).
// If not present, the default class is used (provided on input).
//
// That is:
// op = 'identity' ;    % the GrB_IDENTITY_*, type is default_opclass.
//
// op.opname = 'minv' ; op.class = 'int8' ; % the GrB_MINV_INT8 operator.

#include "GB_mex.h"

bool GB_mx_mxArray_to_UnaryOp          // true if successful
(
    GrB_UnaryOp *handle,                // returns GraphBLAS version of op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name,                   // name of the argument
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const bool XisComplex               // true if X is complex
)
{
    GB_WHERE ("GB_mx_mxArray_to_UnaryOp") ;

    (*handle) = NULL ;
    const mxArray *opname_mx = NULL, *opclass_mx = NULL ;

    if (op_matlab == NULL || mxIsEmpty (op_matlab))
    {
        // op is not present; defaults will be used
        ;
    }
    else if (mxIsStruct (op_matlab))
    {
        // look for op.opname
        int fieldnumber = mxGetFieldNumber (op_matlab, "opname") ;
        if (fieldnumber >= 0)
        {
            opname_mx = mxGetFieldByNumber (op_matlab, 0, fieldnumber) ;
        }
        // look for op.class
        fieldnumber = mxGetFieldNumber (op_matlab, "opclass") ;
        if (fieldnumber >= 0)
        {
            opclass_mx = mxGetFieldByNumber (op_matlab, 0, fieldnumber) ;
        }
    }
    else if (mxIsChar (op_matlab))
    {
        // op is a string.  default class will be used
        opname_mx = op_matlab ;
    }
    else
    {
        mexWarnMsgIdAndTxt ("GB:warn",
            "unrecognized unary op: must be string or struct") ;
        return (false) ;
    }

    // find the corresponding built-in GraphBLAS operator
    GrB_UnaryOp op ;
    if (!GB_mx_string_to_UnaryOp (&op, default_opcode,
        default_opclass, opname_mx, opclass_mx, NULL, NULL, XisComplex))
    {
        mexWarnMsgIdAndTxt ("GB:warn", "unary op failed") ;
        return (false) ;
    }

    // return the op
    ASSERT_UNARYOP_OK (op, name, GB0) ;
    (*handle) = op ;
    return (true) ;
}


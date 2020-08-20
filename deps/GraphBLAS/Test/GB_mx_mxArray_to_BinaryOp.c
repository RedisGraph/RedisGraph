//------------------------------------------------------------------------------
// GB_mx_mxArray_to_BinaryOp
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Convert a MATLAB string or struct to a built-in GraphBLAS BinaryOp.  The
// mxArray is either a struct containing two terms: opname (an operator name),
// and an optional MATLAB string opclass (a string, 'logical', 'double', etc).
// If not present, the default class is used (provided on input).
//
// That is:
// op = 'plus' ;    % the GrB_PLUS_*, type determined by default_opclass.
//
// op.opname = 'plus' ; op.class = 'int8' ; % the GrB_PLUS_INT8 operator.

#include "GB_mex.h"

bool GB_mx_mxArray_to_BinaryOp         // true if successful, false otherwise
(
    GrB_BinaryOp *handle,               // the binary op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name,                   // name of the argument
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const bool XisComplex,
    const bool YisComplex
)
{
    GB_WHERE ("GB_mx_mxArray_to_BinaryOp") ;

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
    else
    {
        // op must be a string.  default class will be used
        opname_mx = op_matlab ;
    }

    // find the corresponding built-in GraphBLAS operator
    GrB_BinaryOp op ;
    if (!GB_mx_string_to_BinaryOp (&op, default_opcode,
        default_opclass, opname_mx, opclass_mx, NULL, NULL,
        XisComplex, YisComplex))
    {
        return (false) ;
    }

    // return the op
    ASSERT_BINARYOP_OK_OR_NULL (op, name, GB0) ;
    (*handle) = op ;
    return (true) ;
}


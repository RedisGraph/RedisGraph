//------------------------------------------------------------------------------
// GB_mx_mxArray_to_BinaryOp
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Convert a MATLAB string or struct to a built-in GraphBLAS BinaryOp.  The
// mxArray is either a struct containing two terms: opname (an operator name),
// and an optional MATLAB string optype (a string, 'logical', 'double', etc).
// If not present, the default type is used (provided on input).
//
// That is:
// op = 'plus' ;    % the GrB_PLUS_*, type determined by default_optype.
//
// op.opname = 'plus' ; op.type = 'int8' ; % the GrB_PLUS_INT8 operator.

#include "GB_mex.h"

bool GB_mx_mxArray_to_BinaryOp          // true if successful, false otherwise
(
    GrB_BinaryOp *op_handle,            // the binary op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name,                   // name of the argument
    const GrB_Type default_optype,      // default operator type
    const bool user_complex             // if true, use user-defined Complex
)
{

    (*op_handle) = NULL ;

    const mxArray *opname_mx = NULL, *optype_mx = NULL ;

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
        // look for op.optype
        fieldnumber = mxGetFieldNumber (op_matlab, "optype") ;
        if (fieldnumber >= 0)
        {
            optype_mx = mxGetFieldByNumber (op_matlab, 0, fieldnumber) ;
        }
    }
    else
    {
        // op must be a string.  default type will be used
        opname_mx = op_matlab ;
    }

    // find the corresponding built-in GraphBLAS operator
    GrB_BinaryOp op ;
    if (!GB_mx_string_to_BinaryOp (&op, default_optype, opname_mx, optype_mx,
        user_complex))
    {
        return (false) ;
    }

    // return the op
    ASSERT_BINARYOP_OK_OR_NULL (op, name, GB0) ;
    (*op_handle) = op ;
    return (true) ;
}


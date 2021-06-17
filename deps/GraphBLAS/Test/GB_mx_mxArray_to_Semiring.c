//------------------------------------------------------------------------------
// GB_mx_mxArray_to_Semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Get a semiring struct from MATLAB and convert it into a GraphBLAS semiring.
//
// The semiring MATLAB struct must contain the following strings:
//
//      multiply    a string with the name of the 'multiply' binary operator.
//      add         a string with the name of the 'add' binary operator.
//                  The operator must be commutative.
//      type        the type of x and y for the multiply operator.
//                  ('logical', 'int8', ... 'double complex').  optional.

#include "GB_mex.h"

bool GB_mx_mxArray_to_Semiring         // true if successful
(
    GrB_Semiring *handle,               // the semiring
    const mxArray *semiring_matlab,     // MATLAB version of semiring
    const char *name,                   // name of the argument
    const GrB_Type default_optype,      // default operator type
    const bool user_complex         // if true, use user-defined Complex op
)
{

    (*handle) = NULL ;
    const mxArray *multiply_mx = NULL, *type_mx = NULL, *add_mx = NULL ;

    if (semiring_matlab == NULL || mxIsEmpty (semiring_matlab))
    {
        // semiring is not present; defaults will be used
        ;
    }
    else if (mxIsStruct (semiring_matlab))
    {
        // look for semiring.multiply
        int fieldnumber = mxGetFieldNumber (semiring_matlab, "multiply") ;
        if (fieldnumber >= 0)
        {
            multiply_mx = mxGetFieldByNumber (semiring_matlab, 0, fieldnumber) ;
        }
        // look for semiring.class
        fieldnumber = mxGetFieldNumber (semiring_matlab, "class") ;
        if (fieldnumber >= 0)
        {
            type_mx = mxGetFieldByNumber (semiring_matlab, 0, fieldnumber) ;
        }
        // look for semiring.add
        fieldnumber = mxGetFieldNumber (semiring_matlab, "add") ;
        if (fieldnumber >= 0)
        {
            add_mx = mxGetFieldByNumber (semiring_matlab, 0, fieldnumber) ;
        }
    }
    else
    {
        mexWarnMsgIdAndTxt ("GB:warn","invalid semiring") ;
        return (false) ;
    }

    // find the corresponding GraphBLAS multiply operator
    GrB_BinaryOp multiply = NULL ;
    if (!GB_mx_string_to_BinaryOp (&multiply, default_optype,
        multiply_mx, type_mx, user_complex) || multiply == NULL)
    {
        mexWarnMsgIdAndTxt ("GB:warn","mult missing or failed") ;
        return (false) ;
    }

    ASSERT_BINARYOP_OK (multiply, "semiring multiply", GB0) ;

    // find the corresponding GraphBLAS add operator
    GrB_BinaryOp add = NULL ;
    if (!GB_mx_string_to_BinaryOp (&add, multiply->ztype,
        add_mx, NULL, user_complex) || add == NULL)
    {
        mexWarnMsgIdAndTxt ("GB:warn", "add missing or failed") ;
        return (false) ;
    }

    ASSERT_BINARYOP_OK (add, "semiring add", GB0) ;
    ASSERT_BINARYOP_OK (multiply, "semiring multiply", GB0) ;

    // create the monoid with the add operator and its identity value
    GrB_Monoid monoid = GB_mx_BinaryOp_to_Monoid (add) ;
    if (monoid == NULL)
    {
        mexWarnMsgIdAndTxt ("GB:warn", "monoid missing or failed") ;
        return (false) ;
    }

    // create the semiring
    GrB_Semiring semiring = GB_mx_semiring (monoid, multiply) ;
    if (semiring == NULL)
    {
        return (false) ;
    }

    ASSERT_SEMIRING_OK (semiring, "semiring", GB0) ;

    (*handle) = semiring ;
    return (true) ;
}


//------------------------------------------------------------------------------
// LAGraph_Vector_isequal: check two vectors for exact equality
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_Vector_isequal, contributed by Tim Davis, Texas A&M

// Checks if two vectors are identically equal (same size,
// type, pattern, size, and values).  Checking for the same type requires the
// GxB_Vector_type function, which is an extension in SuiteSparse:GraphBLAS.
// For the standard API, there is no way to determine the type of a vector.

// See also LAGraph_isequal.

// TODO: add GrB_Vector_Type to the GraphBLAS spec.

// For both methods, if the two vectors are GrB_FP32, GrB_FP64, or
// LAGraph_ComplexFP64, and have NaNs, then these functions will return false,
// since NaN == NaN is false.  To check for NaN equality (like
// isequalwithequalnans in MATLAB), use LAGraph_isall with a user-defined
// operator f(x,y) that returns true if x and y are both NaN.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL ;

GrB_Info LAGraph_Vector_isequal    // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Vector A,
    GrB_Vector B,
    GrB_BinaryOp userop     // for A and B with arbitrary user-defined types.
                            // Ignored if A and B are of built-in types or
                            // LAGraph_ComplexFP64.
)
{

    GrB_Info info ;
    GrB_Type atype, btype ;
    GrB_BinaryOp op ;

    // check inputs
    if (result == NULL)
    {
        // error: required parameter, result, is NULL
        return (GrB_NULL_POINTER) ;
    }
    (*result) = false ;

    // check the type of A and B
    LAGRAPH_OK (GxB_Vector_type (&atype, A)) ;
    LAGRAPH_OK (GxB_Vector_type (&btype, B)) ;
    if (atype != btype)
    {
        // types differ
        return (GrB_SUCCESS) ;
    }

    // select the comparator operator
    if      (atype == GrB_BOOL  ) op = GrB_EQ_BOOL   ;
    else if (atype == GrB_INT8  ) op = GrB_EQ_INT8   ;
    else if (atype == GrB_INT16 ) op = GrB_EQ_INT16  ;
    else if (atype == GrB_INT32 ) op = GrB_EQ_INT32  ;
    else if (atype == GrB_INT64 ) op = GrB_EQ_INT64  ;
    else if (atype == GrB_UINT8 ) op = GrB_EQ_UINT8  ;
    else if (atype == GrB_UINT16) op = GrB_EQ_UINT16 ;
    else if (atype == GrB_UINT32) op = GrB_EQ_UINT32 ;
    else if (atype == GrB_UINT64) op = GrB_EQ_UINT64 ;
    else if (atype == GrB_FP32  ) op = GrB_EQ_FP32   ;
    else if (atype == GrB_FP64  ) op = GrB_EQ_FP64   ;
    else if (atype == LAGraph_ComplexFP64) op = LAGraph_EQ_ComplexFP64 ;
    else op = userop ;

    // check the size, pattern, and values of A and B
    LAGRAPH_OK (LAGraph_Vector_isall (result, A, B, op)) ;

    // return result
    return (GrB_SUCCESS) ;
}


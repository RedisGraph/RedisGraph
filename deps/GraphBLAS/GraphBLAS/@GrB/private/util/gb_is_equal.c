//------------------------------------------------------------------------------
// gb_is_equal: check two matrices for exact equality
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gb_is_equal checks if two matrices are identically equal (same size,
// type, pattern, size, and values).

// If the two matrices are GrB_FP32, GrB_FP64, or gb_complex_type (in the
// future), and have NaNs, then gb_is_equal returns false, since NaN == NaN is
// false.  To check for NaN equality (like isequaln in MATLAB), use gb_is_all
// with a user-defined operator f(x,y) that returns true if x and y are equal,
// or if both are NaN, and false otherwise.

#include "gb_matlab.h"

bool gb_is_equal            // true if A == B, false if A ~= B
(
    GrB_Matrix A,
    GrB_Matrix B
)
{

    // check the type of A and B
    GrB_Type atype, btype ;
    OK (GxB_Matrix_type (&atype, A)) ;
    OK (GxB_Matrix_type (&btype, B)) ;
    if (atype != btype)
    { 
        // types differ
        return (false) ;
    }

    // select the comparator operator
    GrB_BinaryOp op ;
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
    #ifdef GB_COMPLEX_TYPE
    else if (atype == gb_complex_type) op = ... ;
    #endif
    else
    {
        ERROR ("unsupported type") ;
    }

    // check the size, pattern, and values of A and B
    return (gb_is_all (A, B, op)) ;
}


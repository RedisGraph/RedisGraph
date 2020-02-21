//------------------------------------------------------------------------------
// gb_string_and_type_to_unop: get a GraphBLAS operator from a string and type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

// op_name: a MATLAB string defining the operator name (6 kinds):
// 'identity', 'ainv' 'minv', 'lnot', 'one', 'abs'.

// The following equivalent synonyms are available:
//  ainv    -   negate
//  lnot    ~   not
//  one     1

// Total # of ops: 6*11 = 66, not including GrB_LNOT,
// which is equivalent to GxB_LNOT_BOOL.

// FUTURE: add complex unary operators.

GrB_UnaryOp gb_string_and_type_to_unop  // return op from string and type
(
    const char *op_name,        // name of the operator, as a string
    const GrB_Type type         // type of the x,y inputs to the operator
)
{

    CHECK_ERROR (type == NULL, "unsupported type") ;

    if (MATCH (op_name, "identity"))
    { 

        if (type == GrB_BOOL  ) return (GrB_IDENTITY_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_IDENTITY_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_IDENTITY_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_IDENTITY_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_IDENTITY_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_IDENTITY_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_IDENTITY_UINT16) ;
        if (type == GrB_UINT32) return (GrB_IDENTITY_UINT32) ;
        if (type == GrB_UINT64) return (GrB_IDENTITY_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_IDENTITY_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_IDENTITY_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (... ) ;
        #endif

    }
    else if (MATCH (op_name, "ainv") || MATCH (op_name, "-") ||
             MATCH (op_name, "negate"))
    { 

        if (type == GrB_BOOL  ) return (GrB_AINV_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_AINV_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_AINV_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_AINV_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_AINV_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_AINV_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_AINV_UINT16) ;
        if (type == GrB_UINT32) return (GrB_AINV_UINT32) ;
        if (type == GrB_UINT64) return (GrB_AINV_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_AINV_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_AINV_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "minv"))
    { 

        if (type == GrB_BOOL  ) return (GrB_MINV_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_MINV_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_MINV_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_MINV_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_MINV_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_MINV_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_MINV_UINT16) ;
        if (type == GrB_UINT32) return (GrB_MINV_UINT32) ;
        if (type == GrB_UINT64) return (GrB_MINV_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_MINV_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_MINV_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "lnot") || MATCH (op_name, "~") ||
             MATCH (op_name, "not"))
    { 

        if (type == GrB_BOOL  ) return (GxB_LNOT_BOOL  ) ;  // == GrB_LNOT
        if (type == GrB_INT8  ) return (GxB_LNOT_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_LNOT_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_LNOT_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_LNOT_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_LNOT_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_LNOT_UINT16) ;
        if (type == GrB_UINT32) return (GxB_LNOT_UINT32) ;
        if (type == GrB_UINT64) return (GxB_LNOT_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_LNOT_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_LNOT_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "one") || MATCH (op_name, "1"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ONE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ONE_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ONE_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ONE_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ONE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ONE_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ONE_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ONE_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ONE_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ONE_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ONE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "abs"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ABS_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ABS_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ABS_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ABS_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ABS_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ABS_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ABS_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ABS_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ABS_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ABS_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ABS_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }

    ERROR2 ("unknown unary operator", op_name) ;
    return (NULL) ;
}


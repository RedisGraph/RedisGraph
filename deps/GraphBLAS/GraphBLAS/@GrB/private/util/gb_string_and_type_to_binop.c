//------------------------------------------------------------------------------
// gb_string_and_type_to_binop: get a GraphBLAS operator from a string and type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

// op_name: a MATLAB string defining the operator name (25 kinds):
// 11: 1st, 2nd, pair, min, max, +, -, rminus, *, /, \
//  6: iseq, isne, isgt, islt, isge, isle,
//  6: ==, ~=, >, <, >=, <=,
//  3: ||, &&, xor

// The following synonyms are allowed for specifying these operators:
//
//      1st   first
//      2nd   second
//      pair
//      +     plus
//      -     minus
//      *     times
//      /     div
//      \     rdiv
//      ==    eq
//      ~=    ne
//      >     gt
//      <     lt
//      >=    ge
//      <=    le
//      ||    |     or  lor
//      &&    &     and land
//      xor   lxor

// Total # of ops: 26*11 = 286, not including GrB_LOR, GrB_LAND, GrB_XOR,
// which are equivalent to the GxB_*_BOOL versions.

// FUTURE: add complex operators

GrB_BinaryOp gb_string_and_type_to_binop    // return op from string and type
(
    const char *op_name,        // name of the operator, as a string
    const GrB_Type type         // type of the x,y inputs to the operator
)
{

    CHECK_ERROR (type == NULL, "unsupported type") ;

    if (MATCH (op_name, "1st") || MATCH (op_name, "first"))
    { 

        if (type == GrB_BOOL  ) return (GrB_FIRST_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_FIRST_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_FIRST_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_FIRST_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_FIRST_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_FIRST_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_FIRST_UINT16) ;
        if (type == GrB_UINT32) return (GrB_FIRST_UINT32) ;
        if (type == GrB_UINT64) return (GrB_FIRST_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_FIRST_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_FIRST_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (... ) ;
        #endif

    }
    else if (MATCH (op_name, "2nd") || MATCH (op_name, "second"))
    { 

        if (type == GrB_BOOL  ) return (GrB_SECOND_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_SECOND_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_SECOND_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_SECOND_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_SECOND_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_SECOND_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_SECOND_UINT16) ;
        if (type == GrB_UINT32) return (GrB_SECOND_UINT32) ;
        if (type == GrB_UINT64) return (GrB_SECOND_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_SECOND_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_SECOND_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "pair"))
    { 

        if (type == GrB_BOOL  ) return (GxB_PAIR_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_PAIR_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_PAIR_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_PAIR_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_PAIR_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_PAIR_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_PAIR_UINT16) ;
        if (type == GrB_UINT32) return (GxB_PAIR_UINT32) ;
        if (type == GrB_UINT64) return (GxB_PAIR_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_PAIR_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_PAIR_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (... ) ;
        #endif

    }
    else if (MATCH (op_name, "any"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ANY_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ANY_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ANY_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ANY_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ANY_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ANY_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ANY_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ANY_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ANY_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ANY_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ANY_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (... ) ;
        #endif

    }
    else if (MATCH (op_name, "min"))
    { 

        if (type == GrB_BOOL  ) return (GrB_MIN_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_MIN_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_MIN_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_MIN_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_MIN_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_MIN_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_MIN_UINT16) ;
        if (type == GrB_UINT32) return (GrB_MIN_UINT32) ;
        if (type == GrB_UINT64) return (GrB_MIN_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_MIN_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_MIN_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "max"))
    { 

        if (type == GrB_BOOL  ) return (GrB_MAX_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_MAX_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_MAX_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_MAX_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_MAX_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_MAX_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_MAX_UINT16) ;
        if (type == GrB_UINT32) return (GrB_MAX_UINT32) ;
        if (type == GrB_UINT64) return (GrB_MAX_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_MAX_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_MAX_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "+") || MATCH (op_name, "plus"))
    { 

        if (type == GrB_BOOL  ) return (GrB_PLUS_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_PLUS_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_PLUS_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_PLUS_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_PLUS_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_PLUS_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_PLUS_UINT16) ;
        if (type == GrB_UINT32) return (GrB_PLUS_UINT32) ;
        if (type == GrB_UINT64) return (GrB_PLUS_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_PLUS_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_PLUS_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "-") || MATCH (op_name, "minus"))
    { 

        if (type == GrB_BOOL  ) return (GrB_MINUS_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_MINUS_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_MINUS_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_MINUS_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_MINUS_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_MINUS_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_MINUS_UINT16) ;
        if (type == GrB_UINT32) return (GrB_MINUS_UINT32) ;
        if (type == GrB_UINT64) return (GrB_MINUS_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_MINUS_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_MINUS_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "rminus"))
    { 

        if (type == GrB_BOOL  ) return (GxB_RMINUS_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_RMINUS_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_RMINUS_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_RMINUS_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_RMINUS_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_RMINUS_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_RMINUS_UINT16) ;
        if (type == GrB_UINT32) return (GxB_RMINUS_UINT32) ;
        if (type == GrB_UINT64) return (GxB_RMINUS_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_RMINUS_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_RMINUS_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "*") || MATCH (op_name, "times"))
    { 

        if (type == GrB_BOOL  ) return (GrB_TIMES_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_TIMES_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_TIMES_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_TIMES_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_TIMES_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_TIMES_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_TIMES_UINT16) ;
        if (type == GrB_UINT32) return (GrB_TIMES_UINT32) ;
        if (type == GrB_UINT64) return (GrB_TIMES_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_TIMES_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_TIMES_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "/") || MATCH (op_name, "div"))
    { 

        if (type == GrB_BOOL  ) return (GrB_DIV_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_DIV_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_DIV_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_DIV_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_DIV_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_DIV_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_DIV_UINT16) ;
        if (type == GrB_UINT32) return (GrB_DIV_UINT32) ;
        if (type == GrB_UINT64) return (GrB_DIV_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_DIV_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_DIV_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "\\") || MATCH (op_name, "rdiv"))
    { 

        if (type == GrB_BOOL  ) return (GxB_RDIV_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_RDIV_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_RDIV_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_RDIV_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_RDIV_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_RDIV_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_RDIV_UINT16) ;
        if (type == GrB_UINT32) return (GxB_RDIV_UINT32) ;
        if (type == GrB_UINT64) return (GxB_RDIV_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_RDIV_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_RDIV_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "iseq"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ISEQ_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ISEQ_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ISEQ_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ISEQ_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ISEQ_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ISEQ_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ISEQ_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ISEQ_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ISEQ_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ISEQ_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ISEQ_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "isne"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ISNE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ISNE_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ISNE_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ISNE_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ISNE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ISNE_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ISNE_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ISNE_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ISNE_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ISNE_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ISNE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "isgt"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ISGT_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ISGT_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ISGT_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ISGT_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ISGT_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ISGT_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ISGT_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ISGT_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ISGT_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ISGT_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ISGT_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "islt"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ISLT_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ISLT_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ISLT_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ISLT_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ISLT_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ISLT_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ISLT_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ISLT_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ISLT_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ISLT_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ISLT_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "isge"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ISGE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ISGE_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ISGE_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ISGE_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ISGE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ISGE_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ISGE_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ISGE_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ISGE_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ISGE_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ISGE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "isle"))
    { 

        if (type == GrB_BOOL  ) return (GxB_ISLE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GxB_ISLE_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_ISLE_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_ISLE_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_ISLE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_ISLE_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_ISLE_UINT16) ;
        if (type == GrB_UINT32) return (GxB_ISLE_UINT32) ;
        if (type == GrB_UINT64) return (GxB_ISLE_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_ISLE_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_ISLE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "==") || MATCH (op_name, "eq"))
    { 

        if (type == GrB_BOOL  ) return (GrB_EQ_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_EQ_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_EQ_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_EQ_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_EQ_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_EQ_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_EQ_UINT16) ;
        if (type == GrB_UINT32) return (GrB_EQ_UINT32) ;
        if (type == GrB_UINT64) return (GrB_EQ_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_EQ_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_EQ_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "~=") || MATCH (op_name, "ne"))
    { 

        if (type == GrB_BOOL  ) return (GrB_NE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_NE_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_NE_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_NE_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_NE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_NE_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_NE_UINT16) ;
        if (type == GrB_UINT32) return (GrB_NE_UINT32) ;
        if (type == GrB_UINT64) return (GrB_NE_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_NE_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_NE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, ">") || MATCH (op_name, "gt"))
    { 

        if (type == GrB_BOOL  ) return (GrB_GT_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_GT_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_GT_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_GT_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_GT_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_GT_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_GT_UINT16) ;
        if (type == GrB_UINT32) return (GrB_GT_UINT32) ;
        if (type == GrB_UINT64) return (GrB_GT_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_GT_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_GT_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "<") || MATCH (op_name, "lt"))
    { 

        if (type == GrB_BOOL  ) return (GrB_LT_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_LT_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_LT_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_LT_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_LT_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_LT_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_LT_UINT16) ;
        if (type == GrB_UINT32) return (GrB_LT_UINT32) ;
        if (type == GrB_UINT64) return (GrB_LT_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_LT_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_LT_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, ">=") || MATCH (op_name, "ge"))
    { 

        if (type == GrB_BOOL  ) return (GrB_GE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_GE_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_GE_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_GE_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_GE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_GE_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_GE_UINT16) ;
        if (type == GrB_UINT32) return (GrB_GE_UINT32) ;
        if (type == GrB_UINT64) return (GrB_GE_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_GE_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_GE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "<=") || MATCH (op_name, "le"))
    { 

        if (type == GrB_BOOL  ) return (GrB_LE_BOOL  ) ;
        if (type == GrB_INT8  ) return (GrB_LE_INT8  ) ;
        if (type == GrB_INT16 ) return (GrB_LE_INT16 ) ;
        if (type == GrB_INT32 ) return (GrB_LE_INT32 ) ;
        if (type == GrB_INT64 ) return (GrB_LE_INT64 ) ;
        if (type == GrB_UINT8 ) return (GrB_LE_UINT8 ) ;
        if (type == GrB_UINT16) return (GrB_LE_UINT16) ;
        if (type == GrB_UINT32) return (GrB_LE_UINT32) ;
        if (type == GrB_UINT64) return (GrB_LE_UINT64) ;
        if (type == GrB_FP32  ) return (GrB_LE_FP32  ) ;
        if (type == GrB_FP64  ) return (GrB_LE_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "||") || MATCH (op_name, "|")  ||
             MATCH (op_name, "or") || MATCH (op_name, "lor"))
    { 

        if (type == GrB_BOOL  ) return (GrB_LOR       ) ;
        if (type == GrB_INT8  ) return (GxB_LOR_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_LOR_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_LOR_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_LOR_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_LOR_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_LOR_UINT16) ;
        if (type == GrB_UINT32) return (GxB_LOR_UINT32) ;
        if (type == GrB_UINT64) return (GxB_LOR_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_LOR_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_LOR_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "&&")  || MATCH (op_name, "&")   ||
             MATCH (op_name, "and") || MATCH (op_name, "land"))
    { 

        if (type == GrB_BOOL  ) return (GrB_LAND       ) ;
        if (type == GrB_INT8  ) return (GxB_LAND_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_LAND_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_LAND_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_LAND_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_LAND_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_LAND_UINT16) ;
        if (type == GrB_UINT32) return (GxB_LAND_UINT32) ;
        if (type == GrB_UINT64) return (GxB_LAND_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_LAND_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_LAND_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }
    else if (MATCH (op_name, "xor") || MATCH (op_name, "lxor"))
    { 

        if (type == GrB_BOOL  ) return (GrB_LXOR       ) ;
        if (type == GrB_INT8  ) return (GxB_LXOR_INT8  ) ;
        if (type == GrB_INT16 ) return (GxB_LXOR_INT16 ) ;
        if (type == GrB_INT32 ) return (GxB_LXOR_INT32 ) ;
        if (type == GrB_INT64 ) return (GxB_LXOR_INT64 ) ;
        if (type == GrB_UINT8 ) return (GxB_LXOR_UINT8 ) ;
        if (type == GrB_UINT16) return (GxB_LXOR_UINT16) ;
        if (type == GrB_UINT32) return (GxB_LXOR_UINT32) ;
        if (type == GrB_UINT64) return (GxB_LXOR_UINT64) ;
        if (type == GrB_FP32  ) return (GxB_LXOR_FP32  ) ;
        if (type == GrB_FP64  ) return (GxB_LXOR_FP64  ) ;
        #ifdef GB_COMPLEX_TYPE
        if (type == gb_complex_type) return (...) ;
        #endif

    }

    ERROR2 ("unknown binary operator", op_name) ;
    return (NULL) ;
}


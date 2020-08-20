//------------------------------------------------------------------------------
// gb_binop_to_monoid: convert a binary operator to the corresponding monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Monoid gb_binop_to_monoid           // return monoid from a binary op
(
    GrB_BinaryOp op
)
{ 

    GrB_Monoid monoid ;

    // MIN monoids:
         if (op == GrB_MIN_INT8   ) monoid = GxB_MIN_INT8_MONOID ;
    else if (op == GrB_MIN_INT16  ) monoid = GxB_MIN_INT16_MONOID ;
    else if (op == GrB_MIN_INT32  ) monoid = GxB_MIN_INT32_MONOID ;
    else if (op == GrB_MIN_INT64  ) monoid = GxB_MIN_INT64_MONOID ;
    else if (op == GrB_MIN_UINT8  ) monoid = GxB_MIN_UINT8_MONOID ;
    else if (op == GrB_MIN_UINT16 ) monoid = GxB_MIN_UINT16_MONOID ;
    else if (op == GrB_MIN_UINT32 ) monoid = GxB_MIN_UINT32_MONOID ;
    else if (op == GrB_MIN_UINT64 ) monoid = GxB_MIN_UINT64_MONOID ;
    else if (op == GrB_MIN_FP32   ) monoid = GxB_MIN_FP32_MONOID ;
    else if (op == GrB_MIN_FP64   ) monoid = GxB_MIN_FP64_MONOID ;

    // MAX monoids:
    else if (op == GrB_MAX_INT8   ) monoid = GxB_MAX_INT8_MONOID ;
    else if (op == GrB_MAX_INT16  ) monoid = GxB_MAX_INT16_MONOID ;
    else if (op == GrB_MAX_INT32  ) monoid = GxB_MAX_INT32_MONOID ;
    else if (op == GrB_MAX_INT64  ) monoid = GxB_MAX_INT64_MONOID ;
    else if (op == GrB_MAX_UINT8  ) monoid = GxB_MAX_UINT8_MONOID ;
    else if (op == GrB_MAX_UINT16 ) monoid = GxB_MAX_UINT16_MONOID ;
    else if (op == GrB_MAX_UINT32 ) monoid = GxB_MAX_UINT32_MONOID ;
    else if (op == GrB_MAX_UINT64 ) monoid = GxB_MAX_UINT64_MONOID ;
    else if (op == GrB_MAX_FP32   ) monoid = GxB_MAX_FP32_MONOID ;
    else if (op == GrB_MAX_FP64   ) monoid = GxB_MAX_FP64_MONOID ;

    // PLUS monoids:
    else if (op == GrB_PLUS_INT8  ) monoid = GxB_PLUS_INT8_MONOID ;
    else if (op == GrB_PLUS_INT16 ) monoid = GxB_PLUS_INT16_MONOID ;
    else if (op == GrB_PLUS_INT32 ) monoid = GxB_PLUS_INT32_MONOID ;
    else if (op == GrB_PLUS_INT64 ) monoid = GxB_PLUS_INT64_MONOID ;
    else if (op == GrB_PLUS_UINT8 ) monoid = GxB_PLUS_UINT8_MONOID ;
    else if (op == GrB_PLUS_UINT16) monoid = GxB_PLUS_UINT16_MONOID ;
    else if (op == GrB_PLUS_UINT32) monoid = GxB_PLUS_UINT32_MONOID ;
    else if (op == GrB_PLUS_UINT64) monoid = GxB_PLUS_UINT64_MONOID ;
    else if (op == GrB_PLUS_FP32  ) monoid = GxB_PLUS_FP32_MONOID ;
    else if (op == GrB_PLUS_FP64  ) monoid = GxB_PLUS_FP64_MONOID ;

    // TIMES monoids:
    else if (op == GrB_TIMES_INT8  ) monoid = GxB_TIMES_INT8_MONOID ;
    else if (op == GrB_TIMES_INT16 ) monoid = GxB_TIMES_INT16_MONOID ;
    else if (op == GrB_TIMES_INT32 ) monoid = GxB_TIMES_INT32_MONOID ;
    else if (op == GrB_TIMES_INT64 ) monoid = GxB_TIMES_INT64_MONOID ;
    else if (op == GrB_TIMES_UINT8 ) monoid = GxB_TIMES_UINT8_MONOID ;
    else if (op == GrB_TIMES_UINT16) monoid = GxB_TIMES_UINT16_MONOID ;
    else if (op == GrB_TIMES_UINT32) monoid = GxB_TIMES_UINT32_MONOID ;
    else if (op == GrB_TIMES_UINT64) monoid = GxB_TIMES_UINT64_MONOID ;
    else if (op == GrB_TIMES_FP32  ) monoid = GxB_TIMES_FP32_MONOID ;
    else if (op == GrB_TIMES_FP64  ) monoid = GxB_TIMES_FP64_MONOID ;

    // Boolean monoids:
    else if (op == GrB_LOR         ) monoid = GxB_LOR_BOOL_MONOID ;
    else if (op == GrB_LAND        ) monoid = GxB_LAND_BOOL_MONOID ;
    else if (op == GrB_LXOR        ) monoid = GxB_LXOR_BOOL_MONOID ;
    else if (op == GrB_EQ_BOOL     ) monoid = GxB_EQ_BOOL_MONOID ;

    else
    { 
        ERROR ("not a valid monoid") ;
    }

    return (monoid) ;
}


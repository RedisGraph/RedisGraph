//------------------------------------------------------------------------------
// gb_first_binop: return the GrB_FIRST operator for a given type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_BinaryOp gb_first_binop         // return GrB_FIRST_[type] operator
(
    const GrB_Type type
)
{ 

    if      (type == GrB_BOOL)   return (GrB_FIRST_BOOL) ;
    else if (type == GrB_INT8)   return (GrB_FIRST_INT8) ;
    else if (type == GrB_INT16)  return (GrB_FIRST_INT16) ;
    else if (type == GrB_INT32)  return (GrB_FIRST_INT32) ;
    else if (type == GrB_INT64)  return (GrB_FIRST_INT64) ;
    else if (type == GrB_UINT8)  return (GrB_FIRST_UINT8) ;
    else if (type == GrB_UINT16) return (GrB_FIRST_UINT16) ;
    else if (type == GrB_UINT32) return (GrB_FIRST_UINT32) ;
    else if (type == GrB_UINT64) return (GrB_FIRST_UINT64) ;
    else if (type == GrB_FP32)   return (GrB_FIRST_FP32) ;
    else if (type == GrB_FP64)   return (GrB_FIRST_FP64) ;
    #ifdef GB_COMPLEX_TYPE
    else if (type == gb_complex_type)
    {
        return (gb_first_complex) ;
    }
    #endif
    else
    {
        ERROR ("unsupported type") ;
    }

    return (NULL) ;
}


//------------------------------------------------------------------------------
// GxB_Vector_subassign_[SCALAR]: assign scalar to vector, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Assigns a single scalar to a subvector, w(I)<mask> = accum(w(I),x)
// The scalar x is implicitly expanded into a vector u of size ni-by-1,
// with each entry in u equal to x.

#include "GB.h"

#define ASSIGN(type,T,ampersand)                                               \
GrB_Info GxB_Vector_subassign_ ## T /* w(I)<mask> = accum (w(I),x)          */ \
(                                                                              \
    GrB_Vector w,                   /* input/output vector for results      */ \
    const GrB_Vector mask,          /* optional mask for w(I)               */ \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(w(I),x)   */ \
    const type x,                   /* scalar to assign to w(I)             */ \
    const GrB_Index *I,             /* row indices                          */ \
    const GrB_Index ni,             /* number of row indices                */ \
    const GrB_Descriptor desc       /* descriptor for w(I) and mask         */ \
)                                                                              \
{                                                                              \
    WHERE ("GxB_Vector_subassign_" GB_STR(T) " (w, mask, accum, x, I, ni, desc)") ; \
    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;                                      \
    RETURN_IF_UNINITIALIZED (mask) ;                                           \
    return (GB_subassign_scalar ((GrB_Matrix) w, (GrB_Matrix) mask, accum,     \
        ampersand x, GB_## T ## _code, I, ni, GrB_ALL, 1, desc)) ;             \
}

ASSIGN (bool     , BOOL   , &) ;
ASSIGN (int8_t   , INT8   , &) ;
ASSIGN (uint8_t  , UINT8  , &) ;
ASSIGN (int16_t  , INT16  , &) ;
ASSIGN (uint16_t , UINT16 , &) ;
ASSIGN (int32_t  , INT32  , &) ;
ASSIGN (uint32_t , UINT32 , &) ;
ASSIGN (int64_t  , INT64  , &) ;
ASSIGN (uint64_t , UINT64 , &) ;
ASSIGN (float    , FP32   , &) ;
ASSIGN (double   , FP64   , &) ;
ASSIGN (void *   , UDT    ,  ) ;

#undef ASSIGN


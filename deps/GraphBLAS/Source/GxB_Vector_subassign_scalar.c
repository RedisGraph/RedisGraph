//------------------------------------------------------------------------------
// GxB_Vector_subassign_[SCALAR]: assign scalar to vector, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Assigns a single scalar to a subvector, w(Rows)<mask> = accum(w(Rows),x)
// The scalar x is implicitly expanded into a vector u of size nRows-by-1,
// with each entry in u equal to x.

#include "GB.h"

#define GB_ASSIGN(type,T,ampersand)                                            \
GrB_Info GxB_Vector_subassign_ ## T /* w(Rows)<mask> = accum (w(Rows),x)    */ \
(                                                                              \
    GrB_Vector w,                   /* input/output vector for results      */ \
    const GrB_Vector mask,          /* optional mask for w(Rows)            */ \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(w(Rows),x)*/ \
    const type x,                   /* scalar to assign to w(Rows)          */ \
    const GrB_Index *Rows,          /* row indices                          */ \
    GrB_Index nRows,                /* number of row indices                */ \
    const GrB_Descriptor desc       /* descriptor for w(Rows) and mask      */ \
)                                                                              \
{                                                                              \
    GB_WHERE ("GxB_Vector_subassign_" GB_STR(T)                                \
        " (w, mask, accum, x, Rows, nRows, desc)") ;                           \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                          \
    GB_RETURN_IF_FAULTY (mask) ;                                               \
    ASSERT (GB_VECTOR_OK (w)) ;                                                \
    ASSERT (GB_IMPLIES (mask != NULL, GB_VECTOR_OK (mask))) ;                  \
    return (GB_subassign_scalar ((GrB_Matrix) w, (GrB_Matrix) mask, accum,     \
        ampersand x, GB_## T ## _code, Rows, nRows, GrB_ALL, 1, desc,          \
        Context)) ;                                                            \
}

GB_ASSIGN (bool     , BOOL   , &)
GB_ASSIGN (int8_t   , INT8   , &)
GB_ASSIGN (uint8_t  , UINT8  , &)
GB_ASSIGN (int16_t  , INT16  , &)
GB_ASSIGN (uint16_t , UINT16 , &)
GB_ASSIGN (int32_t  , INT32  , &)
GB_ASSIGN (uint32_t , UINT32 , &)
GB_ASSIGN (int64_t  , INT64  , &)
GB_ASSIGN (uint64_t , UINT64 , &)
GB_ASSIGN (float    , FP32   , &)
GB_ASSIGN (double   , FP64   , &)
GB_ASSIGN (void *   , UDT    ,  )


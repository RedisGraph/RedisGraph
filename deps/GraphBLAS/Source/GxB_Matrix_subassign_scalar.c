//------------------------------------------------------------------------------
// GxB_Matrix_subassign_[SCALAR]: assign a scalar to matrix, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Assigns a single scalar to a submatrix, C(I,J)<Mask> = accum(C(I,J),x)
// The scalar x is implicitly expanded into a matrix A of size ni-by-nj,
// with each entry in A equal to x.

#include "GB.h"

#define ASSIGN(type,T,ampersand)                                               \
GrB_Info GxB_Matrix_subassign_ ## T    /* C(I,J)<Mask> = accum (C(I,J),x)      */ \
(                                                                              \
    GrB_Matrix C,                   /* input/output matrix for results      */ \
    const GrB_Matrix Mask,          /* optional mask for C(I,J)             */ \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(C(I,J),x) */ \
    const type x,                   /* scalar to assign to C(I,J)           */ \
    const GrB_Index *I,             /* row indices                          */ \
    const GrB_Index ni,             /* number of row indices                */ \
    const GrB_Index *J,             /* column indices                       */ \
    const GrB_Index nj,             /* number of column indices             */ \
    const GrB_Descriptor desc       /* descriptor for C(I,J) and Mask       */ \
)                                                                              \
{                                                                              \
    WHERE ("GxB_Matrix_subassign_" GB_STR(T) " (C, Mask, accum, x, I, ni, J, nj, desc)") ; \
    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;                                      \
    RETURN_IF_UNINITIALIZED (Mask) ;                                           \
    return (GB_subassign_scalar (C, Mask, accum,                                  \
        ampersand x, GB_## T ## _code, I, ni, J, nj, desc)) ;                  \
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


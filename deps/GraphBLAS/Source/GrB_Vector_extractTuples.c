//------------------------------------------------------------------------------
// GrB_Vector_extractTuples: extract all tuples from a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extracts all tuples from a column, like [I,~,X] = find (v) in MATLAB.  If
// any parameter I and/or X is NULL, then that component is not extracted.  The
// size of the I and X arrays (those that are not NULL) is given by nvals,
// which must be at least as large as GrB_nvals (&nvals, v).  The values in the
// typecasted to the type of X, as needed.

// If any parameter I and/or X is NULL, that component is not extracted.  So to
// extract just the row indices, pass I as non-NULL, and X as NULL.  This is
// like [I,~,~] = find (v) in MATLAB.

#include "GB.h"

#define GB_EXTRACT(prefix,type,T)                                             \
GrB_Info prefix ## Vector_extractTuples_ ## T     /* [I,~,X] = find (A) */    \
(                                                                             \
    GrB_Index *I,           /* array for returning row indices of tuples */   \
    type *X,                /* array for returning values of tuples      */   \
    GrB_Index *p_nvals,     /* I, X size on input; # tuples on output    */   \
    const GrB_Vector v      /* vector to extract tuples from             */   \
)                                                                             \
{                                                                             \
    GB_WHERE1 (GB_STR(prefix) "Vector_extractTuples_" GB_STR(T)          \
        " (I, X, nvals, v)") ;                                                \
    GB_BURBLE_START ("GrB_Vector_extractTuples") ;                            \
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;                                         \
    GB_RETURN_IF_NULL (p_nvals) ;                                             \
    ASSERT (GB_VECTOR_OK (v)) ;                                               \
    GrB_Info info = GB_extractTuples (I, NULL, X, p_nvals, GB_ ## T ## _code, \
        (GrB_Matrix) v, Context) ;                                            \
    GB_BURBLE_END ;                                                           \
    return (info) ;                                                           \
}

GB_EXTRACT (GrB_, bool      , BOOL   )
GB_EXTRACT (GrB_, int8_t    , INT8   )
GB_EXTRACT (GrB_, uint8_t   , UINT8  )
GB_EXTRACT (GrB_, int16_t   , INT16  )
GB_EXTRACT (GrB_, uint16_t  , UINT16 )
GB_EXTRACT (GrB_, int32_t   , INT32  )
GB_EXTRACT (GrB_, uint32_t  , UINT32 )
GB_EXTRACT (GrB_, int64_t   , INT64  )
GB_EXTRACT (GrB_, uint64_t  , UINT64 )
GB_EXTRACT (GrB_, float     , FP32   )
GB_EXTRACT (GrB_, double    , FP64   )
GB_EXTRACT (GxB_, GxB_FC32_t, FC32   )
GB_EXTRACT (GxB_, GxB_FC64_t, FC64   )
GB_EXTRACT (GrB_, void      , UDT    )


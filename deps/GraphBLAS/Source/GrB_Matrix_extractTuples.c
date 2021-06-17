//------------------------------------------------------------------------------
// GrB_Matrix_extractTuples: extract all tuples from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extracts all tuples from a matrix, like [I,J,X] = find (A) in MATLAB.  If
// any parameter I, J and/or X is NULL, then that component is not extracted.
// The size of the I, J, and X arrays (those that are not NULL) is given by
// nvals, which must be at least as large as GrB_nvals (&nvals, A).  The values
// in the matrix are typecasted to the type of X, as needed.

// If any parameter I, J, and/or X is NULL, that component is not extracted.
// So to extract just the row and col indices, pass I and J as non-NULL,
// and X as NULL.  This is like [I,J,~] = find (A).

#include "GB.h"

#define GB_EXTRACT(prefix,type,T)                                             \
GrB_Info prefix ## Matrix_extractTuples_ ## T     /* [I,J,X] = find (A) */    \
(                                                                             \
    GrB_Index *I,           /* array for returning row indices of tuples */   \
    GrB_Index *J,           /* array for returning col indices of tuples */   \
    type *X,                /* array for returning values of tuples      */   \
    GrB_Index *p_nvals,     /* I,J,X size on input; # tuples on output   */   \
    const GrB_Matrix A      /* matrix to extract tuples from             */   \
)                                                                             \
{                                                                             \
    GB_WHERE1 (GB_STR(prefix) "Matrix_extractTuples_" GB_STR(T)          \
        " (I, J, X, nvals, A)") ;                                             \
    GB_BURBLE_START ("GrB_Matrix_extractTuples") ;                            \
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                         \
    GB_RETURN_IF_NULL (p_nvals) ;                                             \
    GrB_Info info = GB_extractTuples (I, J, X, p_nvals, GB_ ## T ## _code, A, \
        Context) ;                                                            \
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


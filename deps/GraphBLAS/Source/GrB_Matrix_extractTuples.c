//------------------------------------------------------------------------------
// GrB_Matrix_extractTuples: extract all tuples from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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

#define GB_EXTRACT(type,T)                                                    \
GrB_Info GrB_Matrix_extractTuples_ ## T     /* [I,J,X] = find (A) */          \
(                                                                             \
    GrB_Index *I,           /* array for returning row indices of tuples */   \
    GrB_Index *J,           /* array for returning col indices of tuples */   \
    type *X,                /* array for returning values of tuples      */   \
    GrB_Index *p_nvals,     /* I,J,X size on input; # tuples on output   */   \
    const GrB_Matrix A      /* matrix to extract tuples from             */   \
)                                                                             \
{                                                                             \
    GB_WHERE ("GrB_Matrix_extractTuples_" GB_STR(T) " (I, J, X, nvals, A)") ; \
    GB_BURBLE_START ("GrB_Matrix_extractTuples") ;                            \
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                         \
    GB_RETURN_IF_NULL (p_nvals) ;                                             \
    GrB_Info info = GB_extractTuples (I, J, X, p_nvals, GB_ ## T ## _code, A, \
        Context) ;                                                            \
    GB_BURBLE_END ;                                                           \
    return (info) ;                                                           \
}

GB_EXTRACT (bool     , BOOL   )
GB_EXTRACT (int8_t   , INT8   )
GB_EXTRACT (uint8_t  , UINT8  )
GB_EXTRACT (int16_t  , INT16  )
GB_EXTRACT (uint16_t , UINT16 )
GB_EXTRACT (int32_t  , INT32  )
GB_EXTRACT (uint32_t , UINT32 )
GB_EXTRACT (int64_t  , INT64  )
GB_EXTRACT (uint64_t , UINT64 )
GB_EXTRACT (float    , FP32   )
GB_EXTRACT (double   , FP64   )
GB_EXTRACT (void     , UDT    )


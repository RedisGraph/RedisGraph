//------------------------------------------------------------------------------
// GrB_Vector_extractTuples: extract all tuples from a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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

#define EXTRACT(type,T) \
GrB_Info GrB_Vector_extractTuples_ ## T     /* [I,~,X] = find (A) */          \
(                                                                             \
    GrB_Index *I,           /* array for returning row indices of tuples */   \
    type *X,                /* array for returning values of tuples      */   \
    GrB_Index *p_nvals,     /* I, X size on input; # tuples on output    */   \
    const GrB_Vector v      /* vector to extract tuples from             */   \
)                                                                             \
{                                                                             \
    WHERE ("GrB_Vector_extractTuples_" GB_STR(T) " (I, X, nvals, v)") ;       \
    RETURN_IF_NULL_OR_UNINITIALIZED (v) ;                                     \
    RETURN_IF_NULL (p_nvals) ;                                                \
    return (GB_extractTuples (I, NULL, X, p_nvals, GB_ ## T ## _code,         \
        (GrB_Matrix) v)) ;                                                    \
}

EXTRACT (bool     , BOOL   ) ;
EXTRACT (int8_t   , INT8   ) ;
EXTRACT (uint8_t  , UINT8  ) ;
EXTRACT (int16_t  , INT16  ) ;
EXTRACT (uint16_t , UINT16 ) ;
EXTRACT (int32_t  , INT32  ) ;
EXTRACT (uint32_t , UINT32 ) ;
EXTRACT (int64_t  , INT64  ) ;
EXTRACT (uint64_t , UINT64 ) ;
EXTRACT (float    , FP32   ) ;
EXTRACT (double   , FP64   ) ;
EXTRACT (void     , UDT    ) ;

#undef EXTRACT


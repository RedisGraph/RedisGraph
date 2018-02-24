//------------------------------------------------------------------------------
// GrB_reduce_to_column: reduce a matrix to a column
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

#define REDUCE(kind,reduceop)                                                 \
GrB_Info GrB_Matrix_reduce_ ## kind /* w<mask> = accum (w,reduce(A))       */ \
(                                                                             \
    GrB_Vector w,                   /* input/output vector for results     */ \
    const GrB_Vector mask,          /* optional mask for w, unused if NULL */ \
    const GrB_BinaryOp accum,       /* optional accum for z=accum(w,t)     */ \
    const GrB_ ## kind reduce,      /* reduce operator for t=reduce(A)     */ \
    const GrB_Matrix A,             /* first input:  matrix A              */ \
    const GrB_Descriptor desc       /* descriptor for w, mask, and A       */ \
)                                                                             \
{                                                                             \
    WHERE ("GrB_Matrix_reduce_" GB_STR(kind) " (w, mask, accum, reduce, A, desc)") ; \
    RETURN_IF_NULL_OR_UNINITIALIZED (reduce) ;                                \
    return (GB_reduce_to_column ((GrB_Matrix) w, (GrB_Matrix) mask, accum,    \
        reduceop, A, desc)) ;                                                 \
}

REDUCE (BinaryOp, reduce    ) ;
REDUCE (Monoid  , reduce->op) ;

#undef REDUCE


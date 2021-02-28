//------------------------------------------------------------------------------
// GrB_reduce_to_vector: reduce a matrix to a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_reduce.h"

#define GB_MATRIX_TO_VECTOR(kind,reduceop,terminal)                           \
GrB_Info GrB_Matrix_reduce_ ## kind /* w<M> = accum (w,reduce(A))          */ \
(                                                                             \
    GrB_Vector w,                   /* input/output vector for results     */ \
    const GrB_Vector M,             /* optional mask for w, unused if NULL */ \
    const GrB_BinaryOp accum,       /* optional accum for z=accum(w,t)     */ \
    const GrB_ ## kind reduce,      /* reduce operator for t=reduce(A)     */ \
    const GrB_Matrix A,             /* first input:  matrix A              */ \
    const GrB_Descriptor desc       /* descriptor for w, M, and A          */ \
)                                                                             \
{                                                                             \
    GB_WHERE ("GrB_Matrix_reduce_" GB_STR(kind)                               \
        " (w, M, accum, reduce, A, desc)") ;                                  \
    GB_BURBLE_START ("GrB_reduce") ;                                          \
    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;                                    \
    GrB_Info info = GB_reduce_to_vector ((GrB_Matrix) w, (GrB_Matrix) M,      \
        accum, reduceop, terminal, A, desc, Context) ;                        \
    GB_BURBLE_END ;                                                           \
    return (info) ;                                                           \
}

// With just a GrB_BinaryOp, built-in operators can terminate early (MIN, MAX,
// LOR, and LAND).  User-defined binary operators do not have a terminal value.
GB_MATRIX_TO_VECTOR (BinaryOp, reduce    , NULL)

// Built-in monoids ignore the terminal parameter, and use the terminal value
// based on the built-in operator.  User-defined monoids can be created with an
// arbitrary non-NULL terminal value.
GB_MATRIX_TO_VECTOR (Monoid  , reduce->op, reduce->terminal)


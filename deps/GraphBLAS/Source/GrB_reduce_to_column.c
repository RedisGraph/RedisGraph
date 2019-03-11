//------------------------------------------------------------------------------
// GrB_reduce_to_column: reduce a matrix to a column
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// parallel: not here, see GB_reduce_to_column

#include "GB.h"

#define GB_REDUCE(kind,reduceop,terminal)                                     \
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
    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;                                    \
    return (GB_reduce_to_column ((GrB_Matrix) w, (GrB_Matrix) M, accum,       \
        reduceop, terminal, A, desc, Context)) ;                              \
}

// With just a GrB_BinaryOp, built-in operators can terminate early (MIN, MAX,
// LOR, and LAND), using the terminal value defined by GB_assoc_template.c.
// User-defined binary operators do not have a terminal value.
GB_REDUCE (BinaryOp, reduce    , NULL)

// Built-in monoids ignore the terminal parameter, and use the terminal value
// defined in GB_assoc_template.c (for MIN, MAX, LOR, and LAND).
// User-defined monoids can be created with a non-NULL terminal value.
GB_REDUCE (Monoid  , reduce->op, reduce->terminal)


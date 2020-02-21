//------------------------------------------------------------------------------
// GB_unaryop:  hard-coded functions for each built-in unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_iterator.h"
#include "GB_unaryop__include.h"

// C=unop(A) is defined by the following types and operators:

// op(A)  function:  GB_unop__lnot_fp64_int64
// op(A') function:  GB_tran__lnot_fp64_int64

// C type:   double
// A type:   int64_t
// cast:     double cij = (double) aij
// unaryop:  cij = !(aij != 0)

#define GB_ATYPE \
    int64_t

#define GB_CTYPE \
    double

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA)  \
    int64_t aij = Ax [pA]

#define GB_CX(p) Cx [p]

// unary operator
#define GB_OP(z, x)   \
    z = !(x != 0) ;

// casting
#define GB_CASTING(z, aij) \
    double z = (double) aij ;

// cij = op (cast (aij))
#define GB_CAST_OP(pC,pA)           \
{                                   \
    /* aij = Ax [pA] */             \
    GB_GETA (aij, Ax, pA) ;         \
    /* Cx [pC] = op (cast (aij)) */ \
    GB_CASTING (z, aij) ;           \
    GB_OP (GB_CX (pC), z) ;         \
}

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_LNOT || GxB_NO_FP64 || GxB_NO_INT64)

//------------------------------------------------------------------------------
// Cx = op (cast (Ax)): apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB_unop__lnot_fp64_int64
(
    double *Cx,       // Cx and Ax may be aliased
    int64_t *Ax,
    int64_t anz,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t p ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    {
        GB_CAST_OP (p, p) ;
    }
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = op (cast (A')): transpose, typecast, and apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB_tran__lnot_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #define GB_PHASE_2_OF_2
    #include "GB_unaryop_transpose.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif


//------------------------------------------------------------------------------
// GB_unop:  hard-coded functions for each built-in unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB.h"
#ifndef GBCUDA_DEV
#include "GB_control.h"
#include "GB_atomics.h"
#include "GB_unop__include.h"

// C=unop(A) is defined by the following types and operators:

// op(A)  function:  GB (_unop_apply__cosh_fp32_fp32)
// op(A') function:  GB (_unop_tran__cosh_fp32_fp32)

// C type:   float
// A type:   float
// cast:     float cij = aij
// unaryop:  cij = coshf (aij)

#define GB_ATYPE \
    float

#define GB_CTYPE \
    float

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA) \
    float aij = Ax [pA]

#define GB_CX(p) Cx [p]

// unary operator
#define GB_OP(z, x) \
    z = coshf (x) ;

// casting
#define GB_CAST(z, aij) \
    float z = aij ;

// cij = op (aij)
#define GB_CAST_OP(pC,pA)           \
{                                   \
    /* aij = Ax [pA] */             \
    float aij = Ax [pA] ;          \
    /* Cx [pC] = op (cast (aij)) */ \
    float z = aij ;               \
    Cx [pC] = coshf (z) ;        \
}

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_COSH || GxB_NO_FP32)

//------------------------------------------------------------------------------
// Cx = op (cast (Ax)): apply a unary operator
//------------------------------------------------------------------------------


GrB_Info GB (_unop_apply__cosh_fp32_fp32)
(
    float *Cx,       // Cx and Ax may be aliased
    const float *Ax,
    const int8_t *restrict Ab,   // A->b if A is bitmap
    int64_t anz,
    int nthreads
)
{
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t p ;
    if (Ab == NULL)
    { 
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        {
            float aij = Ax [p] ;
            float z = aij ;
            Cx [p] = coshf (z) ;
        }
    }
    else
    { 
        // bitmap case, no transpose; A->b already memcpy'd into C->b
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        {
            if (!Ab [p]) continue ;
            float aij = Ax [p] ;
            float z = aij ;
            Cx [p] = coshf (z) ;
        }
    }
    return (GrB_SUCCESS) ;
    #endif
}


//------------------------------------------------------------------------------
// C = op (cast (A')): transpose, typecast, and apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB (_unop_tran__cosh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_unop_transpose.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif


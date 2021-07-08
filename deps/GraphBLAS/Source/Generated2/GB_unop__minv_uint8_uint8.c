//------------------------------------------------------------------------------
// GB_unop:  hard-coded functions for each built-in unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_atomics.h"
#include "GB_unop__include.h"

// C=unop(A) is defined by the following types and operators:

// op(A)  function:  GB (_unop_apply__minv_uint8_uint8)
// op(A') function:  GB (_unop_tran__minv_uint8_uint8)

// C type:   uint8_t
// A type:   uint8_t
// cast:     uint8_t cij = aij
// unaryop:  cij = GB_IMINV_UNSIGNED (aij, 8)

#define GB_ATYPE \
    uint8_t

#define GB_CTYPE \
    uint8_t

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA) \
    uint8_t aij = Ax [pA]

#define GB_CX(p) Cx [p]

// unary operator
#define GB_OP(z, x) \
    z = GB_IMINV_UNSIGNED (x, 8) ;

// casting
#define GB_CAST(z, aij) \
    uint8_t z = aij ;

// cij = op (aij)
#define GB_CAST_OP(pC,pA)           \
{                                   \
    /* aij = Ax [pA] */             \
    uint8_t aij = Ax [pA] ;          \
    /* Cx [pC] = op (cast (aij)) */ \
    uint8_t z = aij ;               \
    Cx [pC] = GB_IMINV_UNSIGNED (z, 8) ;        \
}

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_MINV || GxB_NO_UINT8)

//------------------------------------------------------------------------------
// Cx = op (cast (Ax)): apply a unary operator
//------------------------------------------------------------------------------


GrB_Info GB (_unop_apply__minv_uint8_uint8)
(
    uint8_t *Cx,       // Cx and Ax may be aliased
    const uint8_t *Ax,
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
            uint8_t aij = Ax [p] ;
            uint8_t z = aij ;
            Cx [p] = GB_IMINV_UNSIGNED (z, 8) ;
        }
    }
    else
    { 
        // bitmap case, no transpose; A->b already memcpy'd into C->b
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        {
            if (!Ab [p]) continue ;
            uint8_t aij = Ax [p] ;
            uint8_t z = aij ;
            Cx [p] = GB_IMINV_UNSIGNED (z, 8) ;
        }
    }
    return (GrB_SUCCESS) ;
    #endif
}


//------------------------------------------------------------------------------
// C = op (cast (A')): transpose, typecast, and apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB (_unop_tran__minv_uint8_uint8)
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


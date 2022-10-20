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

// op(A)  function:  GB (_unop_apply__minv_uint16_uint16)
// op(A') function:  GB (_unop_tran__minv_uint16_uint16)

// C type:   uint16_t
// A type:   uint16_t
// cast:     uint16_t cij = aij
// unaryop:  cij = GB_IMINV_UNSIGNED (aij, 16)

#define GB_ATYPE \
    uint16_t

#define GB_CTYPE \
    uint16_t

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA) \
    uint16_t aij = Ax [pA]

#define GB_CX(p) Cx [p]

// unary operator
#define GB_OP(z, x) \
    z = GB_IMINV_UNSIGNED (x, 16) ;

// casting
#define GB_CAST(z, aij) \
    uint16_t z = aij ;

// cij = op (aij)
#define GB_CAST_OP(pC,pA)           \
{                                   \
    /* aij = Ax [pA] */             \
    uint16_t aij = Ax [pA] ;          \
    /* Cx [pC] = op (cast (aij)) */ \
    uint16_t z = aij ;               \
    Cx [pC] = GB_IMINV_UNSIGNED (z, 16) ;        \
}

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_MINV || GxB_NO_UINT16)

//------------------------------------------------------------------------------
// Cx = op (cast (Ax)): apply a unary operator
//------------------------------------------------------------------------------


GrB_Info GB (_unop_apply__minv_uint16_uint16)
(
    uint16_t *Cx,       // Cx and Ax may be aliased
    const uint16_t *Ax,
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
            uint16_t aij = Ax [p] ;
            uint16_t z = aij ;
            Cx [p] = GB_IMINV_UNSIGNED (z, 16) ;
        }
    }
    else
    { 
        // bitmap case, no transpose; A->b already memcpy'd into C->b
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        {
            if (!Ab [p]) continue ;
            uint16_t aij = Ax [p] ;
            uint16_t z = aij ;
            Cx [p] = GB_IMINV_UNSIGNED (z, 16) ;
        }
    }
    return (GrB_SUCCESS) ;
    #endif
}


//------------------------------------------------------------------------------
// C = op (cast (A')): transpose, typecast, and apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB (_unop_tran__minv_uint16_uint16)
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


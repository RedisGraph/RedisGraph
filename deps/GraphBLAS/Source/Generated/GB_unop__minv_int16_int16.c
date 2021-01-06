//------------------------------------------------------------------------------
// GB_unop:  hard-coded functions for each built-in unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_atomics.h"
#include "GB_unop__include.h"

// C=unop(A) is defined by the following types and operators:

// op(A)  function:  GB_unop_apply__minv_int16_int16
// op(A') function:  GB_unop_tran__minv_int16_int16

// C type:   int16_t
// A type:   int16_t
// cast:     int16_t cij = aij
// unaryop:  cij = GB_IMINV_SIGNED (aij, 16)

#define GB_ATYPE \
    int16_t

#define GB_CTYPE \
    int16_t

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA) \
    int16_t aij = Ax [pA]

#define GB_CX(p) Cx [p]

// unary operator
#define GB_OP(z, x) \
    z = GB_IMINV_SIGNED (x, 16) ;

// casting
#define GB_CAST(z, aij) \
    int16_t z = aij ;

// cij = op (aij)
#define GB_CAST_OP(pC,pA)           \
{                                   \
    /* aij = Ax [pA] */             \
    int16_t aij = Ax [pA] ;          \
    /* Cx [pC] = op (cast (aij)) */ \
    int16_t z = aij ;               \
    Cx [pC] = GB_IMINV_SIGNED (z, 16) ;        \
}

// true if operator is the identity op with no typecasting
#define GB_OP_IS_IDENTITY_WITH_NO_TYPECAST \
    0

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_MINV || GxB_NO_INT16)

//------------------------------------------------------------------------------
// Cx = op (cast (Ax)): apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB_unop_apply__minv_int16_int16
(
    int16_t *Cx,       // Cx and Ax may be aliased
    const int16_t *Ax,
    const int8_t *GB_RESTRICT Ab,   // A->b if A is bitmap
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
        #if ( GB_OP_IS_IDENTITY_WITH_NO_TYPECAST )
            GB_memcpy (Cx, Ax, anz * sizeof (int16_t), nthreads) ;
        #else
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < anz ; p++)
            {
                int16_t aij = Ax [p] ;
                int16_t z = aij ;
                Cx [p] = GB_IMINV_SIGNED (z, 16) ;
            }
        #endif
    }
    else
    { 
        // bitmap case, no transpose; A->b already memcpy'd into C->b
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        {
            if (!Ab [p]) continue ;
            int16_t aij = Ax [p] ;
            int16_t z = aij ;
            Cx [p] = GB_IMINV_SIGNED (z, 16) ;
        }
    }
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = op (cast (A')): transpose, typecast, and apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB_unop_tran__minv_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Workspaces,
    const int64_t *GB_RESTRICT A_slice,
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


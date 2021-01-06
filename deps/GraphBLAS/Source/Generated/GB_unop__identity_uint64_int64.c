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

// op(A)  function:  GB_unop_apply__identity_uint64_int64
// op(A') function:  GB_unop_tran__identity_uint64_int64

// C type:   uint64_t
// A type:   int64_t
// cast:     uint64_t cij = (uint64_t) aij
// unaryop:  cij = aij

#define GB_ATYPE \
    int64_t

#define GB_CTYPE \
    uint64_t

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA) \
    int64_t aij = Ax [pA]

#define GB_CX(p) Cx [p]

// unary operator
#define GB_OP(z, x) \
    z = x ;

// casting
#define GB_CAST(z, aij) \
    uint64_t z = (uint64_t) aij ;

// cij = op (aij)
#define GB_CAST_OP(pC,pA)           \
{                                   \
    /* aij = Ax [pA] */             \
    int64_t aij = Ax [pA] ;          \
    /* Cx [pC] = op (cast (aij)) */ \
    uint64_t z = (uint64_t) aij ;               \
    Cx [pC] = z ;        \
}

// true if operator is the identity op with no typecasting
#define GB_OP_IS_IDENTITY_WITH_NO_TYPECAST \
    0

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_IDENTITY || GxB_NO_UINT64 || GxB_NO_INT64)

//------------------------------------------------------------------------------
// Cx = op (cast (Ax)): apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB_unop_apply__identity_uint64_int64
(
    uint64_t *Cx,       // Cx and Ax may be aliased
    const int64_t *Ax,
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
            GB_memcpy (Cx, Ax, anz * sizeof (int64_t), nthreads) ;
        #else
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < anz ; p++)
            {
                int64_t aij = Ax [p] ;
                uint64_t z = (uint64_t) aij ;
                Cx [p] = z ;
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
            int64_t aij = Ax [p] ;
            uint64_t z = (uint64_t) aij ;
            Cx [p] = z ;
        }
    }
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = op (cast (A')): transpose, typecast, and apply a unary operator
//------------------------------------------------------------------------------

GrB_Info GB_unop_tran__identity_uint64_int64
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


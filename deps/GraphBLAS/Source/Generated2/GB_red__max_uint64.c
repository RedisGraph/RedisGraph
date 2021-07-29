//------------------------------------------------------------------------------
// GB_red:  hard-coded functions for reductions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_atomics.h"
#include "GB_control.h" 
#include "GB_red__include.h"

// The reduction is defined by the following types and operators:

// Assemble tuples:    GB (_red_build__max_uint64)
// Reduce to scalar:   GB (_red_scalar__max_uint64)

// A type:   uint64_t
// C type:   uint64_t

// Reduce:   if (aij > s) { s = aij ; }
// Identity: 0
// Terminal: if (s == UINT64_MAX) { break ; }

#define GB_ATYPE \
    uint64_t

#define GB_CTYPE \
    uint64_t

// monoid identity value

    #define GB_IDENTITY \
        0

// declare a scalar and set it equal to the monoid identity value

    #define GB_SCALAR_IDENTITY(s)                   \
        uint64_t s = GB_IDENTITY

// Array to array

    // W [k] = (ztype) S [i], with typecast
    #define GB_CAST_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += (ztype) S [i], with typecast
    #define GB_ADD_CAST_ARRAY_TO_ARRAY(W,k,S,i)     \
        if (S [i] > W [k]) { W [k] = S [i] ; }

    // W [k] += Ax [p], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,Ax,p)         \
        if (Ax [p] > W [k]) { W [k] = Ax [p] ; }  

// Array to scalar

    // s += (ztype) Ax [p], with typecast
    #define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)     \
        if (Ax [p] > s) { s = Ax [p] ; }

    // s += S [i], no typecast
    #define GB_ADD_ARRAY_TO_SCALAR(s,S,i)           \
        if (S [i] > s) { s = S [i] ; }

// Scalar to array

    // W [k] = s, no typecast
    #define GB_COPY_SCALAR_TO_ARRAY(W,k,s)          \
        W [k] = s

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        1

    #define GB_IS_TERMINAL(s)                       \
        (s == UINT64_MAX)

    #define GB_TERMINAL_VALUE                       \
        UINT64_MAX

// panel size for built-in operators

    #define GB_PANEL                                \
        16

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        0

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_MAX || GxB_NO_UINT64 || GxB_NO_MAX_UINT64)

//------------------------------------------------------------------------------
// reduce to a non-iso matrix to scalar, for monoids only
//------------------------------------------------------------------------------


GrB_Info GB (_red_scalar__max_uint64)
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *restrict W_space,
    bool *restrict F,
    int ntasks,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    uint64_t s = (*result) ;
    uint64_t *restrict W = (uint64_t *) W_space ;
    if (A->nzombies > 0 || GB_IS_BITMAP (A))
    {
        #include "GB_reduce_to_scalar_template.c"
    }
    else
    {
        #include "GB_reduce_panel.c"
    }
    (*result) = s ;
    return (GrB_SUCCESS) ;
    #endif
}


//------------------------------------------------------------------------------
// build a non-iso matrix
//------------------------------------------------------------------------------

GrB_Info GB (_red_build__max_uint64)
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict Sx,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_reduce_build_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif


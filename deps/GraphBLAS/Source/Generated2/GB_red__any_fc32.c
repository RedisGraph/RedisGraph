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

// Assemble tuples:    GB (_red_build__any_fc32)
// Reduce to scalar:   GB (_red_scalar__any_fc32)

// A type:   GxB_FC32_t
// C type:   GxB_FC32_t

// Reduce:   s = aij
// Identity: GxB_CMPLXF(0,0)
// Terminal: break ;

#define GB_ATYPE \
    GxB_FC32_t

#define GB_CTYPE \
    GxB_FC32_t

// monoid identity value

    #define GB_IDENTITY \
        GxB_CMPLXF(0,0)

// declare a scalar and set it equal to the monoid identity value

    #define GB_SCALAR_IDENTITY(s)                   \
        GxB_FC32_t s = GB_IDENTITY

// Array to array

    // W [k] = (ztype) S [i], with typecast
    #define GB_CAST_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += (ztype) S [i], with typecast
    #define GB_ADD_CAST_ARRAY_TO_ARRAY(W,k,S,i)     \
        W [k] = S [i]

    // W [k] += Ax [p], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,Ax,p)         \
        W [k] = Ax [p]  

// Array to scalar

    // s += (ztype) Ax [p], with typecast
    #define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)     \
        s = Ax [p]

    // s += S [i], no typecast
    #define GB_ADD_ARRAY_TO_SCALAR(s,S,i)           \
        s = S [i]

// Scalar to array

    // W [k] = s, no typecast
    #define GB_COPY_SCALAR_TO_ARRAY(W,k,s)          \
        W [k] = s

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        1

    #define GB_IS_TERMINAL(s)                       \
        true

    #define GB_TERMINAL_VALUE                       \
        (any value)

// panel size for built-in operators

    #define GB_PANEL                                \
        (no panel)

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        1

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_ANY || GxB_NO_FC32 || GxB_NO_ANY_FC32)

//------------------------------------------------------------------------------
// reduce to a non-iso matrix to scalar, for monoids only
//------------------------------------------------------------------------------


GrB_Info GB (_red_scalar__any_fc32)
(
    GxB_FC32_t *result,
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
    GxB_FC32_t s = (*result) ;
    GxB_FC32_t *restrict W = (GxB_FC32_t *) W_space ;
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

GrB_Info GB (_red_build__any_fc32)
(
    GxB_FC32_t *restrict Tx,
    int64_t  *restrict Ti,
    const GxB_FC32_t *restrict Sx,
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


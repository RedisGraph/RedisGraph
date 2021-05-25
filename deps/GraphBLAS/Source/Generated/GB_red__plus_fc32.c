//------------------------------------------------------------------------------
// GB_red:  hard-coded functions for reductions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_atomics.h"
#include "GB_ek_slice.h"
#include "GB_control.h" 
#include "GB_red__include.h"

// The reduction is defined by the following types and operators:

// Assemble tuples:    GB_red_build__plus_fc32
// Reduce to scalar:   GB_red_scalar__plus_fc32

// A type:   GxB_FC32_t
// C type:   GxB_FC32_t

// Reduce:   s = GB_FC32_add (s, aij)
// Identity: GxB_CMPLXF(0,0)
// Terminal: ;

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
        W [k] = GB_FC32_add (W [k], S [i])

    // W [k] = S [i], no typecast
    #define GB_COPY_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += S [i], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,S,i)          \
        W [k] = GB_FC32_add (W [k], S [i])

// Array to scalar

    // s = (ztype) Ax [p], with typecast
    #define GB_CAST_ARRAY_TO_SCALAR(s,Ax,p)         \
        s = Ax [p]

    // s = W [k], no typecast
    #define GB_COPY_ARRAY_TO_SCALAR(s,W,k)          \
        s = W [k]

    // s += (ztype) Ax [p], with typecast
    #define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)     \
        s = GB_FC32_add (s, Ax [p])

    // s += S [i], no typecast
    #define GB_ADD_ARRAY_TO_SCALAR(s,S,i)           \
        s = GB_FC32_add (s, S [i])

// Scalar to array

    // W [k] = s, no typecast
    #define GB_COPY_SCALAR_TO_ARRAY(W,k,s)          \
        W [k] = s

    // W [k] += s, no typecast
    #define GB_ADD_SCALAR_TO_ARRAY(W,k,s)           \
        W [k] = GB_FC32_add (W [k], s)

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        0

    #define GB_IS_TERMINAL(s)                       \
        (none)

    #define GB_TERMINAL_VALUE                       \
        (none)

    #define GB_BREAK_IF_TERMINAL(s)                 \
        ;

// panel size for built-in operators

    #define GB_PANEL                                \
        32

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        0

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_PLUS || GxB_NO_FC32 || GxB_NO_PLUS_FC32)

//------------------------------------------------------------------------------
// reduce to a scalar, for monoids only
//------------------------------------------------------------------------------



GrB_Info GB_red_scalar__plus_fc32
(
    GxB_FC32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GxB_FC32_t s = (*result) ;
    GxB_FC32_t *GB_RESTRICT W = (GxB_FC32_t *) W_space ;
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
// build matrix
//------------------------------------------------------------------------------

GrB_Info GB_red_build__plus_fc32
(
    GxB_FC32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
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


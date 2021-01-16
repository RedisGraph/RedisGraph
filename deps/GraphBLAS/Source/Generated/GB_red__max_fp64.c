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

// Assemble tuples:    GB_red_build__max_fp64
// Reduce to scalar:   GB_red_scalar__max_fp64

// A type:   double
// C type:   double

// Reduce:   if ((aij > s) || (s != s)) s = aij
// Identity: ((double) -INFINITY)
// Terminal: if (s == ((double) INFINITY)) break ;

#define GB_ATYPE \
    double

#define GB_CTYPE \
    double

// monoid identity value

    #define GB_IDENTITY \
        ((double) -INFINITY)

// declare a scalar and set it equal to the monoid identity value

    #define GB_SCALAR_IDENTITY(s)                   \
        double s = GB_IDENTITY

// Array to array

    // W [k] = (ztype) S [i], with typecast
    #define GB_CAST_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += (ztype) S [i], with typecast
    #define GB_ADD_CAST_ARRAY_TO_ARRAY(W,k,S,i)     \
        if ((S [i] > W [k]) || (W [k] != W [k])) W [k] = S [i]

    // W [k] = S [i], no typecast
    #define GB_COPY_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += S [i], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,S,i)          \
        if ((S [i] > W [k]) || (W [k] != W [k])) W [k] = S [i]

// Array to scalar

    // s = (ztype) Ax [p], with typecast
    #define GB_CAST_ARRAY_TO_SCALAR(s,Ax,p)         \
        s = Ax [p]

    // s = W [k], no typecast
    #define GB_COPY_ARRAY_TO_SCALAR(s,W,k)          \
        s = W [k]

    // s += (ztype) Ax [p], with typecast
    #define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)     \
        if ((Ax [p] > s) || (s != s)) s = Ax [p]

    // s += S [i], no typecast
    #define GB_ADD_ARRAY_TO_SCALAR(s,S,i)           \
        if ((S [i] > s) || (s != s)) s = S [i]

// Scalar to array

    // W [k] = s, no typecast
    #define GB_COPY_SCALAR_TO_ARRAY(W,k,s)          \
        W [k] = s

    // W [k] += s, no typecast
    #define GB_ADD_SCALAR_TO_ARRAY(W,k,s)           \
        if ((s > W [k]) || (W [k] != W [k])) W [k] = s

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        1

    #define GB_IS_TERMINAL(s)                       \
        (s == ((double) INFINITY))

    #define GB_TERMINAL_VALUE                       \
        ((double) INFINITY)

    #define GB_BREAK_IF_TERMINAL(s)                 \
        if (s == ((double) INFINITY)) break ;

// panel size for built-in operators

    #define GB_PANEL                                \
        16

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        0

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_MAX || GxB_NO_FP64 || GxB_NO_MAX_FP64)

//------------------------------------------------------------------------------
// reduce to a scalar, for monoids only
//------------------------------------------------------------------------------



GrB_Info GB_red_scalar__max_fp64
(
    double *result,
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
    double s = (*result) ;
    double *GB_RESTRICT W = (double *) W_space ;
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

GrB_Info GB_red_build__max_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
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


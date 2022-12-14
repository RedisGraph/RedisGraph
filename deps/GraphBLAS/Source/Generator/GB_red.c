//------------------------------------------------------------------------------
// GB_red:  hard-coded functions for reductions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB.h"
#ifndef GBCUDA_DEV
#include "GB_atomics.h"
#include "GB_control.h" 
#include "GB_red__include.h"

// The reduction is defined by the following types and operators:

// Assemble tuples:    GB (_red_build)
// Reduce to scalar:   GB (_red_scalar)

// A type:   GB_atype
// C type:   GB_ctype

// Reduce:   GB_reduce_op(s, aij)
// Identity: GB_identity
// Terminal: GB_terminal

#define GB_ATYPE \
    GB_atype

#define GB_CTYPE \
    GB_ctype

// monoid identity value

    #define GB_IDENTITY \
        GB_identity

// declare a scalar and set it equal to the monoid identity value

    #define GB_SCALAR_IDENTITY(s)                   \
        GB_ctype s = GB_IDENTITY

// Array to array

    // W [k] = (ztype) S [i], with typecast
    #define GB_CAST_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += (ztype) S [i], with typecast
    #define GB_ADD_CAST_ARRAY_TO_ARRAY(W,k,S,i)     \
        GB_reduce_op(W [k], S [i])

    // W [k] += Ax [p], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,Ax,p)         \
        GB_reduce_op(W [k], Ax [p])  

// Array to scalar

    // s += (ztype) Ax [p], with typecast
    #define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)     \
        GB_reduce_op(s, Ax [p])

    // s += S [i], no typecast
    #define GB_ADD_ARRAY_TO_SCALAR(s,S,i)           \
        GB_reduce_op(s, S [i])

// Scalar to array

    // W [k] = s, no typecast
    #define GB_COPY_SCALAR_TO_ARRAY(W,k,s)          \
        W [k] = s

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        GB_has_terminal

    #define GB_IS_TERMINAL(s)                       \
        GB_is_terminal

    #define GB_TERMINAL_VALUE                       \
        GB_terminal_value

// panel size for built-in operators

    #define GB_PANEL                                \
        GB_panel

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        GB_is_any_monoid

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    GB_disable

//------------------------------------------------------------------------------
// reduce to a non-iso matrix to scalar, for monoids only
//------------------------------------------------------------------------------

if_is_monoid
GrB_Info GB (_red_scalar)
(
    GB_atype *result,
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
    GB_ctype s = (*result) ;
    GB_ctype *restrict W = (GB_ctype *) W_space ;
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
endif_is_monoid

//------------------------------------------------------------------------------
// build a non-iso matrix
//------------------------------------------------------------------------------

GrB_Info GB (_red_build)
(
    GB_atype *restrict Tx,
    int64_t  *restrict Ti,
    const GB_atype *restrict Sx,
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


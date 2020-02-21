//------------------------------------------------------------------------------
// GB_red:  hard-coded functions for reductions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_atomics.h"
#include "GB_ek_slice.h"
#include "GB_control.h" 
#include "GB_red__include.h"

// The reduction is defined by the following types and operators:

// Assemble tuples:    GB_red_build__any_fp32
// Reduce to scalar:   GB_red_scalar__any_fp32
// Reduce each vector: GB_red_eachvec__any_fp32
// Reduce each index:  GB_red_eachindex__any_fp32

// A type:   float
// C type:   float

// Reduce:   s = aij
// Identity: 0
// Terminal: break ;

#define GB_ATYPE \
    float

#define GB_CTYPE \
    float

// declare scalar

    #define GB_SCALAR(s)                            \
        float s

// Array to array

    // W [k] = (ztype) S [i], with typecast
    #define GB_CAST_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += (ztype) S [i], with typecast
    #define GB_ADD_CAST_ARRAY_TO_ARRAY(W,k,S,i)     \
        W [k] = S [i]

    // W [k] = S [i], no typecast
    #define GB_COPY_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += S [i], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,S,i)          \
        W [k] = S [i]

// Array to scalar

    // s = (ztype) Ax [p], with typecast
    #define GB_CAST_ARRAY_TO_SCALAR(s,Ax,p)         \
        s = Ax [p]

    // s = W [k], no typecast
    #define GB_COPY_ARRAY_TO_SCALAR(s,W,k)          \
        s = W [k]

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

    // W [k] += s, no typecast
    #define GB_ADD_SCALAR_TO_ARRAY(W,k,s)           \
        W [k] = s

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        1

    #define GB_TERMINAL_VALUE                       \
        (any value)

    #define GB_BREAK_IF_TERMINAL(t)                 \
        break ;

// panel size for built-in operators

    #define GB_PANEL                                \
        (no panel)

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        1

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_ANY || GxB_NO_FP32 || GxB_NO_ANY_FP32)

//------------------------------------------------------------------------------
// reduce to a scalar, for monoids only
//------------------------------------------------------------------------------



GrB_Info GB_red_scalar__any_fp32
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    int ntasks,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    float s = (*result) ;
    #include "GB_reduce_panel.c"
    (*result) = s ;
    return (GrB_SUCCESS) ;
    #endif
}



//------------------------------------------------------------------------------
// reduce to each vector: each vector A(:,k) reduces to a scalar Tx (k)
//------------------------------------------------------------------------------



GrB_Info GB_red_eachvec__any_fp32
(
    float *GB_RESTRICT Tx,
    GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    GB_void *Wfirst_space,
    GB_void *Wlast_space,
    int ntasks,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_reduce_each_vector.c"
    return (GrB_SUCCESS) ;
    #endif
}



//------------------------------------------------------------------------------
// reduce to each index: each A(i,:) reduces to a scalar T (i)
//------------------------------------------------------------------------------



GrB_Info GB_red_eachindex__any_fp32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *GB_RESTRICT pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GrB_Info info = GrB_SUCCESS ;
    GrB_Matrix T = NULL ;
    (*Thandle) = NULL ;
    #define GB_FREE_ALL ;
    #include "GB_reduce_each_index.c"
    (*Thandle) = T ;
    return (info) ;
    #endif
}



//------------------------------------------------------------------------------
// build matrix
//------------------------------------------------------------------------------

GrB_Info GB_red_build__any_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
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


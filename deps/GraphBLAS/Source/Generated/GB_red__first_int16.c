//------------------------------------------------------------------------------
// GB_red:  hard-coded functions for reductions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_ek_slice.h"
#include "GB_control.h" 
#include "GB_red__include.h"

// The reduction is defined by the following types and operators:

// Assemble tuples:    GB_red_build__first_int16
// Reduce to scalar:   GB_red_scalar__(none)
// Reduce each vector: GB_red_eachvec__(none)
// Reduce each index:  GB_red_eachindex__(none)

// A type:   int16_t
// C type:   int16_t

// Reduce:   ;
// Identity: (none)
// Terminal: ;

#define GB_ATYPE \
    int16_t

#define GB_CTYPE \
    int16_t

// declare scalar

    #define GB_SCALAR(s)                            \
        int16_t s

// Array to array

    // W [k] = (ztype) S [i], with typecast
    #define GB_CAST_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += (ztype) S [i], with typecast
    #define GB_ADD_CAST_ARRAY_TO_ARRAY(W,k,S,i)     \
        ;

    // W [k] = S [i], no typecast
    #define GB_COPY_ARRAY_TO_ARRAY(W,k,S,i)         \
        W [k] = S [i]

    // W [k] += S [i], no typecast
    #define GB_ADD_ARRAY_TO_ARRAY(W,k,S,i)          \
        ;

// Array to scalar

    // s = (ztype) Ax [p], with typecast
    #define GB_CAST_ARRAY_TO_SCALAR(s,Ax,p)         \
        s = Ax [p]

    // s = W [k], no typecast
    #define GB_COPY_ARRAY_TO_SCALAR(s,W,k)          \
        s = W [k]

    // s += (ztype) Ax [p], with typecast
    #define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)     \
        ;

    // s += S [i], no typecast
    #define GB_ADD_ARRAY_TO_SCALAR(s,S,i)           \
        ;

// Scalar to array

    // W [k] = s, no typecast
    #define GB_COPY_SCALAR_TO_ARRAY(W,k,s)          \
        W [k] = s

    // W [k] += s, no typecast
    #define GB_ADD_SCALAR_TO_ARRAY(W,k,s)           \
        ;

// workspace

    // declare a ztype array of size ntasks
    #define GB_REDUCTION_WORKSPACE(W,ntasks)        \
        int16_t W [ntasks]   

// break the loop if terminal condition reached

    #define GB_HAS_TERMINAL                         \
        0

    #define GB_TERMINAL_VALUE                       \
        (none)

    #define GB_BREAK_IF_TERMINAL(t)                 \
        ;

// panel size for built-in operators

    #define GB_PANEL                                \
        1

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_FIRST || GxB_NO_INT16 || GxB_NO_FIRST_INT16)

//------------------------------------------------------------------------------
// reduce to a scalar, for monoids only
//------------------------------------------------------------------------------

#if 0

GrB_Info GB_red_scalar__(none)
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int16_t s = (*result) ;
    #include "GB_reduce_panel.c"
    (*result) = s ;
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// reduce to each vector: each vector A(:,k) reduces to a scalar Tx (k)
//------------------------------------------------------------------------------

GrB_Info GB_red_eachvec__(none)
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
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

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
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

#endif

//------------------------------------------------------------------------------
// build matrix
//------------------------------------------------------------------------------

GrB_Info GB_red_build__first_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
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


//------------------------------------------------------------------------------
// GB_AxB_saxpy4: compute C+=A*B: C full, A sparse/hyper, B bitmap/full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_AxB_saxpy4 computes C+=A*B where C is as-if-full, A is
// sparse/hypersparse, and B is bitmap/full (or as-if-full).  No mask is
// present, C_replace is false, the accum matches the monoid, no typecasting is
// needed, and no user-defined types or operators are used.

// The ANY monoid is not supported, since its use as accum would be unusual.
// The monoid must have an atomic implementation, so the TIMES monoid for
// complex types is not supported.

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_control.h"
#ifndef GBCUDA_DEV
#include "GB_AxB__include2.h"
#endif

#define GB_FREE_WORKSPACE               \
{                                       \
    GB_WERK_POP (A_slice, int64_t) ;    \
}

#define GB_FREE_ALL             \
{                               \
    GB_FREE_WORKSPACE ;         \
    GB_phybix_free (C) ;        \
}

//------------------------------------------------------------------------------
// GB_AxB_saxpy4: compute C+=A*B: C full, A sparse/hyper, B bitmap/full
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy4              // C += A*B
(
    GrB_Matrix C,                   // users input/output matrix
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B and accum
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *done_in_place,            // if true, saxpy4 has computed the result
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // saxpy4 is disabled if GraphBLAS is compiled as compact
    //--------------------------------------------------------------------------

    #ifdef GBCUDA_DEV
    return (GrB_NO_VALUE) ;
    #else

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_WERK_DECLARE (A_slice, int64_t) ;

    ASSERT_MATRIX_OK (C, "C for saxpy4 C+=A*B", GB0) ;
    ASSERT (GB_as_if_full (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;

    ASSERT_MATRIX_OK (A, "A for saxpy4 C+=A*B", GB0) ;
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    ASSERT_MATRIX_OK (B, "B for saxpy4 C+=A*B", GB0) ;
    ASSERT (GB_IS_BITMAP (B) || GB_as_if_full (B)) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for saxpy4 C+=A*B", GB0) ;
    ASSERT (A->vdim == B->vlen) ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
//  GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == semiring->add->op->ztype) ;
    bool A_is_pattern, B_is_pattern ;
    GB_binop_pattern (&A_is_pattern, &B_is_pattern, flipxy, mult->opcode) ;

    GB_Opcode mult_binop_code, add_binop_code ;
    GB_Type_code xcode, ycode, zcode ;
    bool builtin_semiring = GB_AxB_semiring_builtin (A, A_is_pattern, B,
        B_is_pattern, semiring, flipxy, &mult_binop_code, &add_binop_code,
        &xcode, &ycode, &zcode) ;

    if (!builtin_semiring || (add_binop_code == GB_ANY_binop_code)
        || (add_binop_code == GB_TIMES_binop_code && (zcode >= GB_FC32_code)))
    { 
        // The semiring must be built-in, and cannot use the ANY monoid.
        // In addition, the TIMES monoid for complex types is not supported,
        // since it cannot be done atomically. 
        return (GrB_NO_VALUE) ;
    }

    GBURBLE ("(saxpy4: %s += %s*%s) ",
            GB_sparsity_char_matrix (C),
            GB_sparsity_char_matrix (A),
            GB_sparsity_char_matrix (B)) ;

    //--------------------------------------------------------------------------
    // ensure C is non-iso
    //--------------------------------------------------------------------------

    GB_OK (GB_convert_any_to_non_iso (C, true, Context)) ;

    //--------------------------------------------------------------------------
    // determine the # of threads to use and the parallel tasks
    //--------------------------------------------------------------------------

    int nthreads, ntasks, nfine_tasks_per_vector ;
    bool use_coarse_tasks, use_atomics ;
    GB_AxB_saxpy4_tasks (&ntasks, &nthreads, &nfine_tasks_per_vector,
        &use_coarse_tasks, &use_atomics, GB_nnz (A), GB_nnz_held (B),
        B->vdim, C->vlen, Context) ;
    if (!use_coarse_tasks)
    {
        // slice the matrix A for each team of fine tasks
        GB_WERK_PUSH (A_slice, nfine_tasks_per_vector + 1, int64_t) ;
        if (A_slice == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
        GB_pslice (A_slice, A->p, A->nvec, nfine_tasks_per_vector, true) ;
    }

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    info = GrB_NO_VALUE ;

    #define GB_Asaxpy4B(add,mult,xname) GB (_Asaxpy4B_ ## add ## mult ## xname)
    #define GB_AxB_WORKER(add,mult,xname)                               \
    {                                                                   \
        info = GB_Asaxpy4B (add,mult,xname) (C, A,                      \
            B, ntasks, nthreads, nfine_tasks_per_vector,                \
            use_coarse_tasks, use_atomics, A_slice, Context) ;          \
    }                                                                   \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    // disabled the ANY monoid, and the TIMES monoid for complex types
    #define GB_NO_ANY_MONOID
    #define GB_NO_NONATOMIC_MONOID
    #include "GB_AxB_factory.c"

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORKSPACE ;
    if (info == GrB_NO_VALUE)
    { 
        // saxpy4 doesn't handle this case; punt to saxpy3, bitmap saxpy, etc
        GBURBLE ("(punt) ") ;
        return (info) ;
    }
    else if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }
    else
    { 
        ASSERT_MATRIX_OK (C, "saxpy4: output", GB0) ;
        (*done_in_place) = true ;
        return (GrB_SUCCESS) ;
    }

    #endif
}


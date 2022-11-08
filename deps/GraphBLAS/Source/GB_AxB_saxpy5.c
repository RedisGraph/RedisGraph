//------------------------------------------------------------------------------
// GB_AxB_saxpy5: compute C+=A*B where A is bitmap/full and B is sparse/hyper
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_AxB_saxpy5 computes C+=A*B where C is as-if-full, A is bitmap/full (or
// as-if-full), and B is sparse/hypersparse.  No mask is present, C_replace is
// false, the accum matches the monoid, no typecasting is needed, and no
// user-defined types or operators are used.

// See also GB_AxB_saxpy4, which computes C+=A*B but with the sparsity formats
// of A and B reversed.

// Typically, if B is a large sparse matrix, the number of columns of A and C
// will be large, so A and C will typically be short-and-fat dense matrices.
// As a result, only a coarse-grain method is used, where no atomics are
// needed.

// The ANY monoid is not supported, since its use as accum would be unusual.
// TODO: if the monoid is ANY, quick return GrB_SUCCESS and done_in_place
// true, also for saxpy4.  No work is needed and C doesn't change.

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_control.h"
#ifndef GBCUDA_DEV
#include "GB_AxB__include2.h"
#endif

#define GB_FREE_WORKSPACE               \
{                                       \
    GB_WERK_POP (B_slice, int64_t) ;    \
}

#define GB_FREE_ALL             \
{                               \
    GB_FREE_WORKSPACE ;         \
    GB_phybix_free (C) ;        \
}

//------------------------------------------------------------------------------
// GB_AxB_saxpy5: compute C+=A*B in-place
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy5              // C += A*B
(
    GrB_Matrix C,                   // users input/output matrix
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B and accum
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *done_in_place,            // if true, saxpy5 has computed the result
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // saxpy5 is disabled if GraphBLAS is compiled as compact
    //--------------------------------------------------------------------------

    #ifdef GBCUDA_DEV
    return (GrB_NO_VALUE) ;
    #else

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_WERK_DECLARE (B_slice, int64_t) ;

    ASSERT_MATRIX_OK (C, "C for saxpy5 C+=A*B", GB0) ;
    ASSERT (GB_as_if_full (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;

    ASSERT_MATRIX_OK (A, "A for saxpy5 C+=A*B", GB0) ;
    ASSERT (GB_IS_BITMAP (A) || GB_as_if_full (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    ASSERT_MATRIX_OK (B, "B for saxpy5 C+=A*B", GB0) ;
    ASSERT (GB_IS_SPARSE (B) || GB_IS_HYPERSPARSE (B)) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for saxpy5 C+=A*B", GB0) ;
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

    if (!builtin_semiring || (add_binop_code == GB_ANY_binop_code))
    { 
        // The semiring must be built-in, and cannot use the ANY monoid.
        return (GrB_NO_VALUE) ;
    }

    GBURBLE ("(saxpy5: %s += %s*%s) ",
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

    int64_t anz = GB_nnz_held (A) ;
    int64_t bnz = GB_nnz_held (B) ;
    int64_t bnvec = B->nvec ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + bnz, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : 4 * nthreads ;
    ntasks = GB_IMIN (ntasks, bnvec) ;
    GB_WERK_PUSH (B_slice, ntasks + 1, int64_t) ;
    if (B_slice == NULL)
    { 
        // out of memory
        GB_FREE_WORKSPACE ;
        return (GrB_OUT_OF_MEMORY) ;
    }
    GB_pslice (B_slice, B->p, bnvec, ntasks, false) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    info = GrB_NO_VALUE ;

    #define GB_Asaxpy5B(add,mult,xname) GB (_Asaxpy5B_ ## add ## mult ## xname)
    #define GB_AxB_WORKER(add,mult,xname)                               \
    {                                                                   \
        info = GB_Asaxpy5B (add,mult,xname) (C, A,                      \
            B, ntasks, nthreads, B_slice, Context) ;                    \
    }                                                                   \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    // disable the ANY monoid
    #define GB_NO_ANY_MONOID
    #include "GB_AxB_factory.c"

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORKSPACE ;
    if (info == GrB_NO_VALUE)
    { 
        // saxpy5 doesn't handle this case; punt to saxpy3, bitmap saxpy, etc
        GBURBLE ("(punt) ") ;
    }
    else if (info == GrB_SUCCESS)
    { 
        ASSERT_MATRIX_OK (C, "saxpy5: output", GB0) ;
        (*done_in_place) = true ;
    }
    return (info) ;
    #endif
}


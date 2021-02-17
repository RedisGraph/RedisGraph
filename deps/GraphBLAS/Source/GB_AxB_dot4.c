//------------------------------------------------------------------------------
// GB_AxB_dot4: compute C+=A'*B in-place
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_AxB_dot4 does its computation in a single phase, computing its result in
// the input matrix C, which is already dense.  The mask M is not handled by
// this function.

#include "GB_mxm.h"
#include "GB_binop.h"
#include "GB_unused.h"
#ifndef GBCOMPACT
#include "GB_AxB__include.h"
#endif

#define GB_FREE_WORK        \
{                           \
    GB_FREE (A_slice) ;     \
    GB_FREE (B_slice) ;     \
}

GrB_Info GB_AxB_dot4                // C+=A'*B, dot product method
(
    GrB_Matrix C,                   // input/output matrix, must be dense
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C+=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for dot in-place += A'*B", GB0) ;
    ASSERT_MATRIX_OK (A, "A for dot in-place += A'*B", GB0) ;
    ASSERT_MATRIX_OK (B, "B for dot in-place += A'*B", GB0) ;
    ASSERT (GB_is_dense (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ;

    ASSERT (!GB_IS_BITMAP (C)) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_BITMAP (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for in-place += A'*B", GB0) ;
    ASSERT (A->vlen == B->vlen) ;

    int64_t *GB_RESTRICT A_slice = NULL ;
    int64_t *GB_RESTRICT B_slice = NULL ;

    GBURBLE ("(%s+=%s'*%s) ",
        GB_sparsity_char_matrix (C),
        GB_sparsity_char_matrix (A),
        GB_sparsity_char_matrix (B)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ_HELD (A) ;
    int64_t bnz = GB_NNZ_HELD (B) ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + bnz, chunk, nthreads_max) ;

    // #include "GB_AxB_dot4_mkl_template.c

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;
    ASSERT (C->type     == add->op->ztype) ;

    bool op_is_first  = mult->opcode == GB_FIRST_opcode ;
    bool op_is_second = mult->opcode == GB_SECOND_opcode ;
    bool op_is_pair   = mult->opcode == GB_PAIR_opcode ;
    bool A_is_pattern = false ;
    bool B_is_pattern = false ;

    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        A_is_pattern = op_is_first  || op_is_pair ;
        B_is_pattern = op_is_second || op_is_pair ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->ytype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->xtype))) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        A_is_pattern = op_is_second || op_is_pair ;
        B_is_pattern = op_is_first  || op_is_pair ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->xtype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->ytype))) ;
    }

    //--------------------------------------------------------------------------
    // slice A and B
    //--------------------------------------------------------------------------

    // A and B can have any sparsity: full, sparse, or hypersparse.
    // C is always full.

    int64_t anvec = A->nvec ;
    int64_t vlen  = A->vlen ;
    int64_t bnvec = B->nvec ;

    int naslice = (nthreads == 1) ? 1 : (16 * nthreads) ;
    int nbslice = (nthreads == 1) ? 1 : (16 * nthreads) ;

    naslice = GB_IMIN (naslice, anvec) ;
    nbslice = GB_IMIN (nbslice, bnvec) ;

    if (!GB_pslice (&A_slice, A->p, anvec, naslice, false)  ||
        !GB_pslice (&B_slice, B->p, bnvec, nbslice, false))
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // C += A'*B, computing each entry with a dot product, via builtin semiring
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_Adot4B(add,mult,xname) GB_Adot4B_ ## add ## mult ## xname

        #define GB_AxB_WORKER(add,mult,xname)           \
        {                                               \
            info = GB_Adot4B (add,mult,xname) (C,       \
                A, A_is_pattern, A_slice, naslice,      \
                B, B_is_pattern, B_slice, nbslice,      \
                nthreads) ;                             \
            done = (info != GrB_NO_VALUE) ;             \
        }                                               \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Opcode mult_opcode, add_opcode ;
        GB_Type_code xcode, ycode, zcode ;

        if (GB_AxB_semiring_builtin (A, A_is_pattern, B, B_is_pattern, semiring,
            flipxy, &mult_opcode, &add_opcode, &xcode, &ycode, &zcode))
        { 
            #include "GB_AxB_factory.c"
        }

    #endif

    //--------------------------------------------------------------------------
    // C += A'*B, computing each entry with a dot product, with typecasting
    //--------------------------------------------------------------------------

    if (!done)
    { 
        #define GB_DOT4_GENERIC
        GB_BURBLE_MATRIX (C, "(generic C+=A'*B) ") ;
        #include "GB_AxB_dot_generic.c"
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "dot4: C += A'*B output", GB0) ;
    return (GrB_SUCCESS) ;
}


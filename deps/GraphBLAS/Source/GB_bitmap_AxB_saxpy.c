//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy: compute C=A*B, C<M>=A*B, or C<!M>=A*B; C bitmap or full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_bitmap_AxB_saxpy.h"
#ifndef GBCOMPACT
#include "GB_AxB__include.h"
#endif

#define GB_FREE_ALL             \
{                               \
    GB_Matrix_free (Chandle) ;  \
}

//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy: compute C=A*B, C<M>=A*B, or C<!M>=A*B
//------------------------------------------------------------------------------

// TODO: also pass in the user's C and the accum operator, and done_in_place,
// like GB_AxB_dot4.

GB_PUBLIC                           // for testing only
GrB_Info GB_bitmap_AxB_saxpy        // C = A*B where C is bitmap or full
(
    GrB_Matrix *Chandle,            // output matrix (not computed in-place)
    const int C_sparsity,
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // mask always applied if present
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    (*mask_applied) = false ;
    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;

    ASSERT_MATRIX_OK_OR_NULL (M, "M for bitmap saxpy A*B", GB0) ;
    ASSERT (!GB_PENDING (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!GB_ZOMBIES (M)) ;

    ASSERT_MATRIX_OK (A, "A for bitmap saxpy A*B", GB0) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    ASSERT_MATRIX_OK (B, "B for bitmap saxpy A*B", GB0) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for bitmap saxpy A*B", GB0) ;
    ASSERT (A->vdim == B->vlen) ;

    ASSERT (C_sparsity == GxB_BITMAP || C_sparsity == GxB_FULL) ;

    //--------------------------------------------------------------------------
    // construct C
    //--------------------------------------------------------------------------

    // TODO: If C is the right type on input, and accum is the same as the
    // monoid, then do not create C, but compute in-place instead.

    GrB_Type ctype = semiring->add->op->ztype ;
    int64_t cnzmax ;
    bool ok = GB_Index_multiply ((GrB_Index *) &cnzmax, A->vlen, B->vdim) ;
    if (!ok)
    { 
        // problem too large
        return (GrB_OUT_OF_MEMORY) ;
    }
    GB_OK (GB_new_bix (Chandle, ctype, A->vlen, B->vdim, GB_Ap_null, true,
        C_sparsity, true, GB_HYPER_SWITCH_DEFAULT, -1, cnzmax, true, Context)) ;
    GrB_Matrix C = *Chandle ;
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;
    bool A_is_pattern, B_is_pattern ;
    GB_AxB_pattern (&A_is_pattern, &B_is_pattern, flipxy, mult->opcode) ;

    //--------------------------------------------------------------------------
    // C<#M>+=A*B
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_Asaxpy3B(add,mult,xname) \
            GB_Asaxpy3B_ ## add ## mult ## xname

        #define GB_AxB_WORKER(add,mult,xname)                               \
        {                                                                   \
            info = GB_Asaxpy3B (add,mult,xname) (C, M, Mask_comp,           \
                Mask_struct, true, A, A_is_pattern,  B,                     \
                B_is_pattern, NULL, 0, 0, 0, 0, Context) ;                  \
            done = (info != GrB_NO_VALUE) ;                                 \
        }                                                                   \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Opcode mult_opcode, add_opcode ;
        GB_Type_code xcode, ycode, zcode ;
        if (GB_AxB_semiring_builtin (A, A_is_pattern, B,
            B_is_pattern, semiring, flipxy, &mult_opcode, &add_opcode, &xcode,
            &ycode, &zcode))
        { 
            #include "GB_AxB_factory.c"
        }

    #endif

    //--------------------------------------------------------------------------
    // generic method
    //--------------------------------------------------------------------------

    if (!done)
    { 
        info = GB_AxB_saxpy_generic (C, M, Mask_comp, Mask_struct,
            true, A, A_is_pattern, B, B_is_pattern, semiring,
            flipxy, NULL, 0, 0, 0, 0, Context) ;
    }

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*mask_applied) = (M != NULL) ;
    ASSERT_MATRIX_OK (C, "C bitmap saxpy output", GB0) ;
    return (GrB_SUCCESS) ;
}


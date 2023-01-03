//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy: compute C=A*B, C<M>=A*B, or C<!M>=A*B; C bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_bitmap_AxB_saxpy.h"
#include "GB_AxB_saxpy_generic.h"
#include "GB_AxB__include1.h"
#ifndef GBCUDA_DEV
#include "GB_AxB__include2.h"
#endif

#define GB_FREE_ALL GB_phybix_free (C) ;

//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy: compute C=A*B, C<M>=A*B, or C<!M>=A*B
//------------------------------------------------------------------------------

// TODO: also pass in the user's C and the accum operator, and done_in_place,
// like GB_AxB_dot4.

GB_PUBLIC                           // for testing only
GrB_Info GB_bitmap_AxB_saxpy        // C = A*B where C is bitmap
(
    GrB_Matrix C,                   // output matrix, static header
    const bool C_iso,               // true if C is iso
    const GB_void *cscalar,         // iso value of C
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;

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

    //--------------------------------------------------------------------------
    // construct C
    //--------------------------------------------------------------------------

    // TODO: If C is the right type on input, and accum is the same as the
    // monoid, then do not create C, but compute in-place instead.

    // Cb is set to all zero.  C->x is malloc'd unless C is iso, in which case
    // it is calloc'ed.

    GrB_Type ctype = semiring->add->op->ztype ;
    int64_t cnzmax = 1 ;
    (void) GB_int64_multiply ((GrB_Index *) &cnzmax, A->vlen, B->vdim) ;
    // set C->iso = C_iso   OK
    GB_OK (GB_new_bix (&C, // existing header
        ctype, A->vlen, B->vdim, GB_Ap_null, true, GxB_BITMAP, true,
        GB_HYPER_SWITCH_DEFAULT, -1, cnzmax, true, C_iso, Context)) ;
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
//  GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == semiring->add->op->ztype) ;
    bool A_is_pattern, B_is_pattern ;
    GB_binop_pattern (&A_is_pattern, &B_is_pattern, flipxy, mult->opcode) ;

    //--------------------------------------------------------------------------
    // C<#M>=A*B
    //--------------------------------------------------------------------------

    if (C_iso)
    { 

        //----------------------------------------------------------------------
        // C is iso; compute the pattern of C<#>=A*B with the any_pair semiring
        //----------------------------------------------------------------------

        GBURBLE ("(iso bitmap saxpy) ") ;
        memcpy (C->x, cscalar, ctype->size) ;
        info = GB (_AsaxbitB__any_pair_iso) (C, M, Mask_comp, Mask_struct, A,
            B, Context) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C is non-iso
        //----------------------------------------------------------------------

        GBURBLE ("(bitmap saxpy) ") ;
        bool done = false ;

        #ifndef GBCUDA_DEV

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_AsaxbitB(add,mult,xname)  \
                GB (_AsaxbitB_ ## add ## mult ## xname)

            #define GB_AxB_WORKER(add,mult,xname)                       \
            {                                                           \
                info = GB_AsaxbitB (add,mult,xname) (C, M, Mask_comp,   \
                    Mask_struct, A, B, Context) ;                       \
                done = (info != GrB_NO_VALUE) ;                         \
            }                                                           \
            break ;

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            GB_Opcode mult_binop_code, add_binop_code ;
            GB_Type_code xcode, ycode, zcode ;
            if (GB_AxB_semiring_builtin (A, A_is_pattern, B, B_is_pattern,
                semiring, flipxy, &mult_binop_code, &add_binop_code, &xcode,
                &ycode, &zcode))
            { 
                #include "GB_AxB_factory.c"
            }

        #endif

        //----------------------------------------------------------------------
        // generic method
        //----------------------------------------------------------------------

        if (!done)
        { 
            info = GB_AxB_saxpy_generic (C, M, Mask_comp, Mask_struct,
                true, A, A_is_pattern, B, B_is_pattern, semiring,
                flipxy, GB_SAXPY_METHOD_BITMAP,
                NULL, 0, 0, 0, 0,
                Context) ;
        }
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

    ASSERT_MATRIX_OK (C, "C bitmap saxpy output", GB0) ;
    return (GrB_SUCCESS) ;
}


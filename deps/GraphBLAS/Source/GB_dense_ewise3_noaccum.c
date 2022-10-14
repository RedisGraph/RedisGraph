//------------------------------------------------------------------------------
// GB_dense_ewise3_noaccum: C = A+B where A and B are dense, C is anything
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C can have any sparsity on input; it becomes a full non-iso matrix on output.
// C can have pending work, which is discarded.

#include "GB_dense.h"
#include "GB_binop.h"
#include "GB_stringify.h"
#ifndef GBCUDA_DEV
#include "GB_binop__include.h"
#endif

#define GB_FREE_ALL ;

GrB_Info GB_dense_ewise3_noaccum    // C = A+B
(
    GrB_Matrix C,                   // input/output matrix
    const bool C_as_if_full,        // true if C is as-if-full on input
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_BinaryOp op,          // must not be a positional op
    GB_Context Context
)
{
#ifndef GBCUDA_DEV

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    ASSERT_MATRIX_OK (C, "C for dense C=A+B", GB0) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;    // C is entirely overwritten by A+B
    ASSERT (GB_PENDING_OK (C)) ;

    ASSERT_MATRIX_OK (A, "A for dense C=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    ASSERT_MATRIX_OK (B, "B for dense C=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ;

    ASSERT (GB_as_if_full (A)) ;
    ASSERT (GB_as_if_full (B)) ;

    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_BITMAP (B)) ;

    ASSERT (!A->iso) ;
    ASSERT (!B->iso) ;

    ASSERT_BINARYOP_OK (op, "op for dense C=A+B", GB0) ;
    ASSERT (!GB_OP_IS_POSITIONAL (op)) ;
    ASSERT (op->ztype == C->type) ;
    ASSERT (op->xtype == A->type) ;
    ASSERT (op->ytype == B->type) ;

    #ifdef GB_DEBUGIFY_DEFN
    GB_debugify_ewise (false, GxB_FULL, C->type, NULL,
        false, false, op, false, A, B) ;
    #endif

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t anz = GB_nnz (A) ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (2 * anz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // if C not already full, allocate it as full
    //--------------------------------------------------------------------------

    // clear prior content and create C as a full matrix.  Keep the same type
    // and CSR/CSC for C.  Allocate the values of C but do not initialize them.

    if (!C_as_if_full)
    { 
        // free the content of C and reallocate it as a non-iso full matrix
        ASSERT (C != A && C != B) ;
        GB_phybix_free (C) ;
        // set C->iso = false   OK
        GB_OK (GB_new_bix (&C,  // existing header
            C->type, C->vlen, C->vdim, GB_Ap_null, C->is_csc, GxB_FULL, false,
            C->hyper_switch, -1, GB_nnz_full (C), true, false, Context)) ;
        C->magic = GB_MAGIC ;
    }
    else if (!GB_IS_FULL (C))
    { 
        // ensure C is full
        GB_convert_any_to_full (C) ;
    }
    ASSERT (GB_IS_FULL (C)) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_Cdense_ewise3_noaccum(op,xname) \
        GB (_Cdense_ewise3_noaccum_ ## op ## xname)

    #define GB_BINOP_WORKER(op,xname)                                       \
    {                                                                       \
        GB_Cdense_ewise3_noaccum(op,xname) (C, A, B, nthreads) ;            \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    GB_Opcode opcode ;
    GB_Type_code xcode, ycode, zcode ;
    if (GB_binop_builtin (A->type, false, B->type, false,
        op, false, &opcode, &xcode, &ycode, &zcode))
    { 
        #include "GB_binop_factory.c"
    }
    else
    {
        // this function is not called if the op cannot be applied
        ASSERT (GB_DEAD_CODE) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C=A+B output", GB0) ;
    return (GrB_SUCCESS) ;
#else
    return (GrB_NO_VALUE) ;
#endif
}


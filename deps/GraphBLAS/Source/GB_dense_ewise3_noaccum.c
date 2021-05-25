//------------------------------------------------------------------------------
// GB_dense_ewise3_noaccum: C = A+B where A and B are dense, C is anything
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C can have any sparsity on input; it becomes a full matrix on output.

#include "GB_dense.h"
#include "GB_binop.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"

#define GB_FREE_ALL ;

GrB_Info GB_dense_ewise3_noaccum    // C = A+B
(
    GrB_Matrix C,                   // input/output matrix
    const bool C_is_dense,          // true if C is dense on input
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_BinaryOp op,          // must not be a positional op
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    ASSERT_MATRIX_OK (C, "C for dense C=A+B", GB0) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;    // C is entirely overwritten by A+B
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (GB_IMPLIES (!C_is_dense, (C != A && C != B))) ;

    ASSERT_MATRIX_OK (A, "A for dense C=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_is_dense (A)) ;

    ASSERT_MATRIX_OK (B, "B for dense C=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (GB_is_dense (B)) ;

    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_BITMAP (B)) ;

    ASSERT_BINARYOP_OK (op, "op for dense C=A+B", GB0) ;
    ASSERT (!GB_OP_IS_POSITIONAL (op)) ;
    ASSERT (op->ztype == C->type) ;
    ASSERT (op->xtype == A->type) ;
    ASSERT (op->ytype == B->type) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (2 * anz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // if C not already dense, allocate it as full
    //--------------------------------------------------------------------------

    // clear prior content and create C as a full matrix.  Keep the same type
    // and CSR/CSC for C.  Allocate the values of C but do not initialize them.

    if (!C_is_dense)
    { 
        // convert C to full; just allocate C->x.  Keep the dimensions of C.
        GB_OK (GB_convert_to_full (C)) ;    // prior content deleted
    }
    else if (!GB_IS_FULL (C))
    {
        // C is dense, but not full; convert to full
        GB_convert_any_to_full (C) ;
    }
    ASSERT (GB_IS_FULL (C)) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_Cdense_ewise3_noaccum(op,xname) \
        GB_Cdense_ewise3_noaccum_ ## op ## xname

    #define GB_BINOP_WORKER(op,xname)                                       \
    {                                                                       \
        info = GB_Cdense_ewise3_noaccum(op,xname) (C, A, B, nthreads) ;     \
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
}

#endif


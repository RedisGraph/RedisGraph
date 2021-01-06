//------------------------------------------------------------------------------
// GB_dense_ewise3_accum: C += A+B where all 3 matries are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_dense.h"
#include "GB_binop.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"

void GB_dense_ewise3_accum          // C += A+B, all matrices dense
(
    GrB_Matrix C,                   // input/output matrix
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_BinaryOp op,          // only GB_BINOP_SUBSET operators supported
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C for dense C+=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (GB_is_dense (C)) ;

    ASSERT_MATRIX_OK (A, "A for dense C+=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_is_dense (A)) ;

    ASSERT_MATRIX_OK (B, "B for dense C+=A+B", GB0) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (GB_is_dense (B)) ;

    ASSERT (!GB_IS_BITMAP (C)) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_BITMAP (B)) ;

    ASSERT_BINARYOP_OK (op, "op for dense C+=A+B", GB0) ;
    ASSERT (!GB_OP_IS_POSITIONAL (op)) ;
    ASSERT (op->ztype == C->type) ;
    ASSERT (op->ztype == A->type) ;
    ASSERT (op->ztype == B->type) ;
    ASSERT (op->ztype == op->xtype) ;
    ASSERT (op->ztype == op->ytype) ;
    ASSERT (op->opcode >= GB_MIN_opcode) ;
    ASSERT (op->opcode <= GB_RDIV_opcode) ;

    GB_ENSURE_FULL (C) ;        // convert C to full

    // FUTURE::: handle IS*, LOR, LAND, LXOR operators

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t cnz = GB_NNZ (C) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (3 * cnz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_Cdense_ewise3_accum(op,xname) \
        GB_Cdense_ewise3_accum_ ## op ## xname

    #define GB_BINOP_WORKER(op,xname)                                       \
    {                                                                       \
        GB_Cdense_ewise3_accum(op,xname) (C, A, B, nthreads) ;              \
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
        #define GB_BINOP_SUBSET
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

    ASSERT_MATRIX_OK (C, "C+=A+B output", GB0) ;
}

#endif


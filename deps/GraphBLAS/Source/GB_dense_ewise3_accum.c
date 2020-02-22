//------------------------------------------------------------------------------
// GB_dense_ewise3_accum: C += A+B where all 3 matries are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GBCOMPACT
#include "GB_dense.h"
#include "GB_binop__include.h"

void GB_dense_ewise3_accum          // C += A+B, all matrices dense
(
    GrB_Matrix C,                   // input/output matrix
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_BinaryOp op,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C for dense C+=A+B", GB0) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (GB_is_dense (C)) ;
    ASSERT (GB_is_dense (A)) ;
    ASSERT (GB_is_dense (B)) ;
    ASSERT_BINARYOP_OK (op, "op for dense C+=A+B", GB0) ;
    ASSERT (op->ztype == C->type) ;
    ASSERT (op->ztype == A->type) ;
    ASSERT (op->ztype == B->type) ;
    ASSERT (op->ztype == op->xtype) ;
    ASSERT (op->ztype == op->ytype) ;
    ASSERT (op->opcode >= GB_MIN_opcode) ;
    ASSERT (op->opcode <= GB_RDIV_opcode) ;

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

    #define GB_Cdense_ewise3_accum(op,xyname) \
        GB_Cdense_ewise3_accum_ ## op ## xyname

    #define GB_BINOP_WORKER(op,xyname)                                      \
    {                                                                       \
        GB_Cdense_ewise3_accum(op,xyname) (C, A, B, nthreads) ;             \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    GB_Opcode opcode ;
    GB_Type_code xycode, zcode ;
    if (GB_binop_builtin (A->type, false, B->type, false, op, false,
        &opcode, &xycode, &zcode))
    { 
        #define GB_BINOP_SUBSET
        #include "GB_binop_factory.c"
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C+=A+B output", GB0) ;
}

#endif


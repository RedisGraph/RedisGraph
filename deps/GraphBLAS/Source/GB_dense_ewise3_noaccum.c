//------------------------------------------------------------------------------
// GB_dense_ewise3_noaccum: C = A+B where A and B are dense, C is anything
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// FUTURE: extend to handle typecasting and generic operators.

#ifndef GBCOMPACT
#include "GB_dense.h"
#include "GB_binop__include.h"

#define GB_FREE_ALL ;

GrB_Info GB_dense_ewise3_noaccum    // C = A+B
(
    GrB_Matrix C,                   // input/output matrix
    const bool C_is_dense,          // true if C is dense
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_BinaryOp op,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for dense C=A+B", GB0) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (GB_IMPLIES (!C_is_dense, (C != A && C != B))) ;
    ASSERT (GB_is_dense (A)) ;
    ASSERT (GB_is_dense (B)) ;
    ASSERT_BINARYOP_OK (op, "op for dense C=A+B", GB0) ;
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
    // if C not already dense, allocate it and create its pattern (same as A)
    //--------------------------------------------------------------------------

    // clear prior content and then create a copy of the pattern of A.  Keep
    // the same type and CSR/CSC for C.  Allocate the values of C but do not
    // initialize them.

    if (!C_is_dense)
    { 
        bool C_is_csc = C->is_csc ;
        GB_PHIX_FREE (C) ;
        GB_OK (GB_dup2 (&C, A, false, C->type, Context)) ;
        C->is_csc = C_is_csc ;
    }

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_Cdense_ewise3_noaccum(op,xyname) \
        GB_Cdense_ewise3_noaccum_ ## op ## xyname

    #define GB_BINOP_WORKER(op,xyname)                                      \
    {                                                                       \
        info = GB_Cdense_ewise3_noaccum(op,xyname) (C, A, B, nthreads) ;    \
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
        #include "GB_binop_factory.c"
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C=A+B output", GB0) ;
    return (GrB_SUCCESS) ;
}

#endif


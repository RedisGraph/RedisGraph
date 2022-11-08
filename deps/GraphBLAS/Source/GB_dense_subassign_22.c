//------------------------------------------------------------------------------
// GB_dense_subassign_22: C += b where C is dense and b is a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C += b where C is a dense or full matrix and b is a scalar
// C can have any sparsity format, as long as all entries are present;
// GB_is_dense (C)) must hold.

#include "GB_dense.h"
#include "GB_binop.h"
#include "GB_unused.h"
#ifndef GBCUDA_DEV
#include "GB_binop__include.h"
#endif

#define GB_FREE_ALL ;

GrB_Info GB_dense_subassign_22      // C += b where C is dense and b is a scalar 
(
    GrB_Matrix C,                   // input/output matrix
    const void *scalar,             // input scalar
    const GrB_Type btype,           // type of the input scalar
    const GrB_BinaryOp accum,       // operator to apply
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for C+=b", GB0) ;
    ASSERT (GB_as_if_full (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;

    ASSERT (scalar != NULL) ;
    ASSERT_TYPE_OK (btype, "btype for C+=b", GB0) ;
    ASSERT_BINARYOP_OK (accum, "accum for C+=b", GB0) ;
    ASSERT (!GB_OP_IS_POSITIONAL (accum)) ;

    GB_ENSURE_FULL (C) ;    // convert C to full, if sparsity control allows it

    //--------------------------------------------------------------------------
    // get the operator
    //--------------------------------------------------------------------------

    if (accum->opcode == GB_FIRST_binop_code || C->iso)
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    // C = accum (C,b) will be computed
    ASSERT (C->type == accum->ztype) ;
    ASSERT (C->type == accum->xtype) ;
    ASSERT (GB_Type_compatible (btype, accum->ytype)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t cnz = GB_nnz (C) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (cnz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // typecast the scalar into the same type as the y input of the binary op
    //--------------------------------------------------------------------------

    int64_t csize = C->type->size ;
    size_t ysize = accum->ytype->size ;
    GB_cast_function 
        cast_B_to_Y = GB_cast_factory (accum->ytype->code, btype->code) ;
    GB_void bwork [GB_VLA(ysize)] ;
    cast_B_to_Y (bwork, scalar, btype->size) ;

    //--------------------------------------------------------------------------
    // C += b, scalar accum into dense, with built-in binary operators
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCUDA_DEV

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_Cdense_accumb(accum,xname) \
            GB (_Cdense_accumb_ ## accum ## xname)

        #define GB_BINOP_WORKER(accum,xname)                                \
        {                                                                   \
            info = GB_Cdense_accumb(accum,xname) (C, bwork, nthreads) ;     \
            done = (info != GrB_NO_VALUE) ;                                 \
        }                                                                   \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Opcode opcode ;
        GB_Type_code xcode, ycode, zcode ;
        if (GB_binop_builtin (C->type, false, btype, false, // C = C + b
            accum, false, &opcode, &xcode, &ycode, &zcode))
        { 
            // accumulate sparse matrix into dense matrix with built-in operator
            #include "GB_binop_factory.c"
        }

    #endif

    //--------------------------------------------------------------------------
    // C += b, scalar accum into dense, with typecasting or user-defined op
    //--------------------------------------------------------------------------

    if (!done)
    { 
        GB_BURBLE_MATRIX (C, "(generic C(:,:)+=x assign) ") ;

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of b and C
        //----------------------------------------------------------------------

        GxB_binary_function fadd = accum->binop_function ;

        //----------------------------------------------------------------------
        // C += b via function pointers, and typecasting
        //----------------------------------------------------------------------

        // C(i,j) = C(i,j) + scalar
        #define GB_BINOP(cout_ij, cin_aij, bwork, i, j) \
            fadd (cout_ij, cin_aij, bwork)

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_SIMD_VECTORIZE ;

        #include "GB_dense_subassign_22_template.c"
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C+=b output", GB0) ;
    return (GrB_SUCCESS) ;
}


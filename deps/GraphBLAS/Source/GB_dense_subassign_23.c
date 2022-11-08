//------------------------------------------------------------------------------
// GB_dense_subassign_23: C += B where C is dense and B is sparse or dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C and B must have the same vector dimension and vector length.
// FUTURE::: the transposed case, C+=B' could easily be done.
// The parallelism used is identical to GB_AxB_colscale.

// The type of C must match the type of x and z for the accum function, since
// C(i,j) = accum (C(i,j), B(i,j)) is handled.  The generic case here can
// typecast B(i,j) but not C(i,j).  The case for typecasting of C is handled by
// Method 04.

// The caller passes in the second matrix as A, but it is called B here to
// match its use as the 2nd input to the binary accum operator.  C and B can
// have any sparsity structure, but C must be dense.

#include "GB_dense.h"
#include "GB_binop.h"
#ifndef GBCUDA_DEV
#include "GB_binop__include.h"
#endif
#include "GB_unused.h"

#define GB_FREE_ALL                         \
{                                           \
    GB_WERK_POP (B_ek_slicing, int64_t) ;   \
}

GrB_Info GB_dense_subassign_23      // C += B; C is dense, B is sparse or dense
(
    GrB_Matrix C,                   // input/output matrix
    const GrB_Matrix B,             // input matrix
    const GrB_BinaryOp accum,       // operator to apply
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_aliased (C, B)) ;   // NO ALIAS of C==A (A is called B here)

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for C+=B", GB0) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (GB_is_dense (C)) ;

    ASSERT_MATRIX_OK (B, "B for C+=B", GB0) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;

    ASSERT_BINARYOP_OK (accum, "accum for C+=B", GB0) ;
    ASSERT (!GB_OP_IS_POSITIONAL (accum)) ;
    ASSERT (B->vlen == C->vlen) ;
    ASSERT (B->vdim == C->vdim) ;

    GB_ENSURE_FULL (C) ;    // convert C to full, if sparsity control allows it

    //--------------------------------------------------------------------------
    // get the operator
    //--------------------------------------------------------------------------

    if (accum->opcode == GB_FIRST_binop_code || C->iso)
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    // C = accum (C,B) will be computed
    ASSERT (C->type == accum->ztype) ;
    ASSERT (C->type == accum->xtype) ;
    ASSERT (GB_Type_compatible (B->type, accum->ytype)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    GB_WERK_DECLARE (B_ek_slicing, int64_t) ;
    int B_ntasks, B_nthreads ;

    if (GB_IS_BITMAP (B) || GB_as_if_full (B))
    { 
        // C is dense and B is bitmap or as-if-full
        GBURBLE ("(Z bitmap/as-if-full) ") ;
        int64_t bnvec = B->nvec ;
        int64_t bnz = GB_nnz_held (B) ;
        B_nthreads = GB_nthreads (bnz + bnvec, chunk, nthreads_max) ;
        B_ntasks = 0 ;   // unused
        ASSERT (B_ek_slicing == NULL) ;
    }
    else
    { 
        // create tasks to compute over the matrix B
        GB_SLICE_MATRIX (B, 32, chunk) ;
        ASSERT (B_ek_slicing != NULL) ;
    }

    //--------------------------------------------------------------------------
    // C += B, sparse accum into dense, with built-in binary operators
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCUDA_DEV

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_Cdense_accumB(accum,xname) \
            GB (_Cdense_accumB_ ## accum ## xname)

        #define GB_BINOP_WORKER(accum,xname)                    \
        {                                                       \
            info = GB_Cdense_accumB(accum,xname) (C, B,         \
                B_ek_slicing, B_ntasks, B_nthreads) ;           \
            done = (info != GrB_NO_VALUE) ;                     \
        }                                                       \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Opcode opcode ;
        GB_Type_code xcode, ycode, zcode ;
        if (GB_binop_builtin (C->type, false, B->type, false, // C = C + B
            accum, false, &opcode, &xcode, &ycode, &zcode))
        { 
            // accumulate sparse matrix into dense matrix with built-in operator
            #include "GB_binop_factory.c"
        }

    #endif

    //--------------------------------------------------------------------------
    // C += B, sparse accum into dense, with typecasting or user-defined op
    //--------------------------------------------------------------------------

    if (!done)
    { 

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of B and C
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (B, "(generic C+=B) ") ;

        GxB_binary_function fadd = accum->binop_function ;

        size_t csize = C->type->size ;
        size_t bsize = B->type->size ;
        size_t ysize = accum->ytype->size ;

        GB_cast_function cast_B_to_Y ;

        // B is typecasted to y
        cast_B_to_Y = GB_cast_factory (accum->ytype->code, B->type->code) ;

        //----------------------------------------------------------------------
        // C += B via function pointers, and typecasting
        //----------------------------------------------------------------------

        // bij = B(i,j), located in Bx [pB].  Note that GB_GETB is used,
        // since B appears as the 2nd input to z = fadd (x,y)
        #define GB_GETB(bij,Bx,pB,B_iso)                                    \
            GB_void bij [GB_VLA(ysize)] ;                                   \
            cast_B_to_Y (bij, Bx +(B_iso ? 0:(pB)*bsize), bsize)

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_SIMD_VECTORIZE ;

        #define GB_BINOP(z,x,y,i,j) fadd (z,x,y)
        #include "GB_dense_subassign_23_template.c"
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    ASSERT_MATRIX_OK (C, "C+=B output", GB0) ;
    return (GrB_SUCCESS) ;
}


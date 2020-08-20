//------------------------------------------------------------------------------
// GB_dense_subassign_22: C += x where C is dense and x is a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C += x where C is a dense matrix and x is a scalar

#include "GB_dense.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"
#endif

GrB_Info GB_dense_subassign_22      // C += x where C is dense and x is a scalar 
(
    GrB_Matrix C,                   // input/output matrix
    const GB_void *scalar,          // input scalar
    const GrB_Type atype,           // type of the input scalar
    const GrB_BinaryOp accum,       // operator to apply
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for C+=x", GB0) ;
    ASSERT (scalar != NULL) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (GB_is_dense (C)) ;
    ASSERT_TYPE_OK (atype, "atype for C+=x", GB0) ;
    ASSERT_BINARYOP_OK (accum, "accum for C+=x", GB0) ;

    //--------------------------------------------------------------------------
    // get the operator
    //--------------------------------------------------------------------------

    if (accum->opcode == GB_FIRST_opcode)
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    // C = accum (C,x) will be computed
    ASSERT (C->type == accum->ztype) ;
    ASSERT (C->type == accum->xtype) ;
    ASSERT (GB_Type_compatible (atype, accum->ytype)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t cnz = GB_NNZ (C) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (cnz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // typecast the scalar into the same type as Y
    //--------------------------------------------------------------------------

    int64_t csize = C->type->size ;
    size_t ysize = accum->ytype->size ;
    GB_cast_function 
        cast_A_to_Y = GB_cast_factory (accum->ytype->code, atype->code) ;
    GB_void ywork [GB_VLA(ysize)] ;
    cast_A_to_Y (ywork, scalar, atype->size) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    bool done = false ;

    #define GB_Cdense_accumX(accum,xyname) GB_Cdense_accumX_ ## accum ## xyname

    #define GB_BINOP_WORKER(accum,xyname)                                   \
    {                                                                       \
        info = GB_Cdense_accumX(accum,xyname) (C, ywork, nthreads) ;        \
        done = (info != GrB_NO_VALUE) ;                                     \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        GB_Opcode opcode ;
        GB_Type_code xycode, zcode ;
        if (GB_binop_builtin (C->type, false, atype, false, accum, false,
            &opcode, &xycode, &zcode))
        { 
            // accumulate sparse matrix into dense matrix with built-in operator
            #include "GB_binop_factory.c"
        }

    #endif

    //--------------------------------------------------------------------------
    // C += x, scalar accum into dense, with typecasting or user-defined op
    //--------------------------------------------------------------------------

    if (!done)
    { 
        GB_BURBLE_MATRIX (C, "generic ") ;

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of x and C
        //----------------------------------------------------------------------

        GxB_binary_function fadd = accum->function ;

        //----------------------------------------------------------------------
        // C += x via function pointers, and typecasting
        //----------------------------------------------------------------------

        // C(i,j) = C(i,j) + scalar
        #define GB_BINOP(cout_ij, cin_aij, ywork) \
            GB_BINARYOP (cout_ij, cin_aij, ywork)

        // binary operator
        #define GB_BINARYOP(z,x,y) fadd (z,x,y)

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_VECTORIZE

        #include "GB_dense_subassign_22_template.c"
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C+=x output", GB0) ;
    return (GrB_SUCCESS) ;
}


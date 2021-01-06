//------------------------------------------------------------------------------
// GB_AxB_colscale: C = A*D where D is diagonal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_binop.h"
#include "GB_apply.h"
#include "GB_ek_slice.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"
#endif

#define GB_FREE_WORK \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;

GrB_Info GB_AxB_colscale            // C = A*D, column scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*D
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (Chandle != NULL) ;
    ASSERT_MATRIX_OK (A, "A for colscale A*D", GB0) ;
    ASSERT_MATRIX_OK (D, "D for colscale A*D", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (D)) ;
    ASSERT (!GB_JUMBLED (D)) ;
    ASSERT (!GB_PENDING (D)) ;
    ASSERT_SEMIRING_OK (semiring, "semiring for numeric A*D", GB0) ;
    ASSERT (A->vdim == D->vlen) ;
    ASSERT (GB_is_diagonal (D, Context)) ;

    ASSERT (!GB_IS_BITMAP (A)) ;        // TODO: ok for now
    ASSERT (!GB_IS_BITMAP (D)) ;
    ASSERT (!GB_IS_FULL (D)) ;

    GBURBLE ("(%s=%s*%s) ",
        GB_sparsity_char_matrix (A),    // C has the sparsity structure of A
        GB_sparsity_char_matrix (A),
        GB_sparsity_char_matrix (D)) ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    ASSERT (mult->ztype == semiring->add->op->ztype) ;
    GB_Opcode opcode = mult->opcode ;
    // GB_reduce_to_vector does not use GB_AxB_colscale:
    ASSERT (!(mult->function == NULL &&
        (opcode == GB_FIRST_opcode || opcode == GB_SECOND_opcode))) ;

    //--------------------------------------------------------------------------
    // copy the pattern of A into C
    //--------------------------------------------------------------------------

    // allocate C->x but do not initialize it
    (*Chandle) = NULL ;
    info = GB_dup (Chandle, A, false, mult->ztype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }
    GrB_Matrix C = (*Chandle) ;

    //--------------------------------------------------------------------------
    // apply a positional operator: convert C=A*D to C=op(A)
    //--------------------------------------------------------------------------

    if (GB_OPCODE_IS_POSITIONAL (opcode))
    { 
        if (flipxy)
        { 
            // the multiplicative operator is fmult(y,x), so flip the opcode
            opcode = GB_binop_flip (opcode) ;
        }
        // determine unary operator to compute C=A*D
        GrB_UnaryOp op1 = NULL ;
        if (mult->ztype == GrB_INT64)
        {
            switch (opcode)
            {
                // first_op(A,D) becomes position_op(A)
                case GB_FIRSTI_opcode   : op1 = GxB_POSITIONI_INT64  ;
                    break ;
                case GB_FIRSTJ_opcode   : op1 = GxB_POSITIONJ_INT64  ;
                    break ;
                case GB_FIRSTI1_opcode  : op1 = GxB_POSITIONI1_INT64 ;
                    break ;
                case GB_FIRSTJ1_opcode  : op1 = GxB_POSITIONJ1_INT64 ;
                    break ;
                // second_op(A,D) becomes position_j(A)
                case GB_SECONDI_opcode  : 
                case GB_SECONDJ_opcode  : op1 = GxB_POSITIONJ_INT64  ;
                    break ;
                case GB_SECONDI1_opcode : 
                case GB_SECONDJ1_opcode : op1 = GxB_POSITIONJ1_INT64 ;
                    break ;
                default:  ;
            }
        }
        else
        {
            switch (opcode)
            {
                // first_op(A,D) becomes position_op(A)
                case GB_FIRSTI_opcode   : op1 = GxB_POSITIONI_INT32  ;
                    break ;
                case GB_FIRSTJ_opcode   : op1 = GxB_POSITIONJ_INT32  ;
                    break ;
                case GB_FIRSTI1_opcode  : op1 = GxB_POSITIONI1_INT32 ;
                    break ;
                case GB_FIRSTJ1_opcode  : op1 = GxB_POSITIONJ1_INT32 ;
                    break ;
                // second_op(A,D) becomes position_j(A)
                case GB_SECONDI_opcode  : 
                case GB_SECONDJ_opcode  : op1 = GxB_POSITIONJ_INT32  ;
                    break ;
                case GB_SECONDI1_opcode : 
                case GB_SECONDJ1_opcode : op1 = GxB_POSITIONJ1_INT32 ;
                    break ;
                default:  ;
            }
        }
        info = GB_apply_op (C->x, op1,      // positional unary op only
            NULL, NULL, false, A, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_Matrix_free (Chandle) ;
            return (info) ;
        }
        ASSERT_MATRIX_OK (C, "colscale positional: C = A*D output", GB0) ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ_HELD (A) ;
    int64_t anvec = A->nvec ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + anvec, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (32 * nthreads) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, A, &ntasks))
    { 
        // out of memory
        GB_Matrix_free (Chandle) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // determine if the values are accessed
    //--------------------------------------------------------------------------

    bool op_is_first  = (opcode == GB_FIRST_opcode) ;
    bool op_is_second = (opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = (opcode == GB_PAIR_opcode) ;
    bool A_is_pattern = false ;
    bool D_is_pattern = false ;

    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        A_is_pattern = op_is_first  || op_is_pair ;
        D_is_pattern = op_is_second || op_is_pair ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->ytype))) ;
        ASSERT (GB_IMPLIES (!D_is_pattern,
            GB_Type_compatible (D->type, mult->xtype))) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        A_is_pattern = op_is_second || op_is_pair ;
        D_is_pattern = op_is_first  || op_is_pair ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->xtype))) ;
        ASSERT (GB_IMPLIES (!D_is_pattern,
            GB_Type_compatible (D->type, mult->ytype))) ;
    }

    //--------------------------------------------------------------------------
    // C = A*D, column scale, via built-in binary operators
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_AxD(mult,xname) GB_AxD_ ## mult ## xname

        #define GB_BINOP_WORKER(mult,xname)                                  \
        {                                                                    \
            info = GB_AxD(mult,xname) (C, A, A_is_pattern, D, D_is_pattern,  \
                kfirst_slice, klast_slice, pstart_slice, ntasks, nthreads) ; \
            done = (info != GrB_NO_VALUE) ;                                  \
        }                                                                    \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Type_code xcode, ycode, zcode ;
        if (GB_binop_builtin (A->type, A_is_pattern, D->type, D_is_pattern,
            mult, flipxy, &opcode, &xcode, &ycode, &zcode))
        { 
            // C=A*D, colscale with built-in operator
            #define GB_BINOP_IS_SEMIRING_MULTIPLIER
            #include "GB_binop_factory.c"
            #undef  GB_BINOP_IS_SEMIRING_MULTIPLIER
        }

    #endif

    //--------------------------------------------------------------------------
    // C = A*D, column scale, with typecasting or user-defined operator
    //--------------------------------------------------------------------------

    if (!done)
    {

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A, D, and C
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (C, "(generic C=A*D colscale) ") ;

        GxB_binary_function fmult = mult->function ;

        size_t csize = C->type->size ;
        size_t asize = A_is_pattern ? 0 : A->type->size ;
        size_t dsize = D_is_pattern ? 0 : D->type->size ;

        size_t xsize = mult->xtype->size ;
        size_t ysize = mult->ytype->size ;

        // scalar workspace: because of typecasting, the x/y types need not
        // be the same as the size of the A and D types.
        // flipxy false: aij = (xtype) A(i,j) and djj = (ytype) D(j,j)
        // flipxy true:  aij = (ytype) A(i,j) and djj = (xtype) D(j,j)
        size_t aij_size = flipxy ? ysize : xsize ;
        size_t djj_size = flipxy ? xsize : ysize ;

        GB_void *GB_RESTRICT Cx = (GB_void *) C->x ;

        GB_cast_function cast_A, cast_D ;
        if (flipxy)
        { 
            // A is typecasted to y, and D is typecasted to x
            cast_A = A_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, A->type->code) ;
            cast_D = D_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, D->type->code) ;
        }
        else
        { 
            // A is typecasted to x, and D is typecasted to y
            cast_A = A_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, A->type->code) ;
            cast_D = D_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, D->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = A*D via function pointers, and typecasting
        //----------------------------------------------------------------------

        // aij = A(i,j), located in Ax [pA]
        #define GB_GETA(aij,Ax,pA)                                          \
            GB_void aij [GB_VLA(aij_size)] ;                                \
            if (!A_is_pattern) cast_A (aij, Ax +((pA)*asize), asize) ;

        // dji = D(j,j), located in Dx [j]
        #define GB_GETB(djj,Dx,j)                                           \
            GB_void djj [GB_VLA(djj_size)] ;                                \
            if (!D_is_pattern) cast_D (djj, Dx +((j)*dsize), dsize) ;

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_SIMD_VECTORIZE ;

        if (flipxy)
        { 
            #define GB_BINOP(z,x,y,i,j) fmult (z,y,x)
            #include "GB_AxB_colscale_meta.c"
            #undef GB_BINOP
        }
        else
        { 
            #define GB_BINOP(z,x,y,i,j) fmult (z,x,y)
            #include "GB_AxB_colscale_meta.c"
            #undef GB_BINOP
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "colscale: C = A*D output", GB0) ;
    ASSERT (*Chandle == C) ;
    GB_FREE_WORK ;
    return (GrB_SUCCESS) ;
}


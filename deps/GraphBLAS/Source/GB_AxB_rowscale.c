//------------------------------------------------------------------------------
// GB_AxB_rowscale: C = D*B, row scale with diagonal matrix D
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_binop.h"
#include "GB_apply.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"
#endif

GrB_Info GB_AxB_rowscale            // C = D*B, row scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=D*A
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (Chandle != NULL) ;

    ASSERT_MATRIX_OK (D, "D for rowscale A*D", GB0) ;
    ASSERT (!GB_ZOMBIES (D)) ;
    ASSERT (!GB_JUMBLED (D)) ;
    ASSERT (!GB_PENDING (D)) ;

    ASSERT_MATRIX_OK (B, "B for rowscale A*D", GB0) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;
    ASSERT (!GB_PENDING (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for numeric D*A", GB0) ;
    ASSERT (D->vdim == B->vlen) ;
    ASSERT (GB_is_diagonal (D, Context)) ;

    ASSERT (!GB_IS_BITMAP (D)) ;        // bitmap or full: not needed
    ASSERT (!GB_IS_BITMAP (B)) ;
    ASSERT (!GB_IS_FULL (D)) ;

    GBURBLE ("(%s=%s*%s) ",
        GB_sparsity_char_matrix (B),    // C has the sparsity structure of B
        GB_sparsity_char_matrix (D),
        GB_sparsity_char_matrix (B)) ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    ASSERT (mult->ztype == semiring->add->op->ztype) ;
    GB_Opcode opcode = mult->opcode ;
    // GB_reduce_to_vector does not use GB_AxB_rowscale:
    ASSERT (!(mult->function == NULL &&
        (opcode == GB_FIRST_opcode || opcode == GB_SECOND_opcode))) ;

    //--------------------------------------------------------------------------
    // copy the pattern of B into C
    //--------------------------------------------------------------------------

    // allocate C->x but do not initialize it
    (*Chandle) = NULL ;
    info = GB_dup (Chandle, B, false, mult->ztype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }
    GrB_Matrix C = (*Chandle) ;

    //--------------------------------------------------------------------------
    // apply a positional operator: convert C=D*B to C=op(B)
    //--------------------------------------------------------------------------

    if (GB_OPCODE_IS_POSITIONAL (opcode))
    { 
        if (flipxy)
        { 
            // the multiplicatve operator is fmult(y,x), so flip the opcode
            opcode = GB_binop_flip (opcode) ;
        }
        // determine unary operator to compute C=D*B
        GrB_UnaryOp op1 = NULL ;
        if (mult->ztype == GrB_INT64)
        {
            switch (opcode)
            {
                // first_op(D,B) becomes position_i(B)
                case GB_FIRSTI_opcode   : 
                case GB_FIRSTJ_opcode   : op1 = GxB_POSITIONI_INT64  ; 
                    break ;
                case GB_FIRSTI1_opcode  : 
                case GB_FIRSTJ1_opcode  : op1 = GxB_POSITIONI1_INT64 ; 
                    break ;
                // second_op(D,B) becomes position_op(B)
                case GB_SECONDI_opcode  : op1 = GxB_POSITIONI_INT64  ; 
                    break ;
                case GB_SECONDJ_opcode  : op1 = GxB_POSITIONJ_INT64  ; 
                    break ;
                case GB_SECONDI1_opcode : op1 = GxB_POSITIONI1_INT64 ; 
                    break ;
                case GB_SECONDJ1_opcode : op1 = GxB_POSITIONJ1_INT64 ; 
                    break ;
                default:  ;
            }
        }
        else
        {
            switch (opcode)
            {
                // first_op(D,B) becomes position_i(B)
                case GB_FIRSTI_opcode   : 
                case GB_FIRSTJ_opcode   : op1 = GxB_POSITIONI_INT32  ; 
                    break ;
                case GB_FIRSTI1_opcode  : 
                case GB_FIRSTJ1_opcode  : op1 = GxB_POSITIONI1_INT32 ; 
                    break ;
                // second_op(D,B) becomes position_op(B)
                case GB_SECONDI_opcode  : op1 = GxB_POSITIONI_INT32  ; 
                    break ;
                case GB_SECONDJ_opcode  : op1 = GxB_POSITIONJ_INT32  ; 
                    break ;
                case GB_SECONDI1_opcode : op1 = GxB_POSITIONI1_INT32 ; 
                    break ;
                case GB_SECONDJ1_opcode : op1 = GxB_POSITIONJ1_INT32 ; 
                    break ;
                default:  ;
            }
        }
        info = GB_apply_op (C->x, op1,      // positional unary op only
            NULL, NULL, false, B, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_Matrix_free (Chandle) ;
            return (info) ;
        }
        ASSERT_MATRIX_OK (C, "rowscale positional: C = D*B output", GB0) ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (GB_NNZ_HELD (B) + B->nvec, chunk, nthreads_max);

    //--------------------------------------------------------------------------
    // determine if the values are accessed
    //--------------------------------------------------------------------------

    bool op_is_first  = (opcode == GB_FIRST_opcode) ;
    bool op_is_second = (opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = (opcode == GB_PAIR_opcode) ;
    bool D_is_pattern = false ;
    bool B_is_pattern = false ;

    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        D_is_pattern = op_is_first  || op_is_pair ;
        B_is_pattern = op_is_second || op_is_pair ;
        ASSERT (GB_IMPLIES (!D_is_pattern,
            GB_Type_compatible (D->type, mult->ytype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->xtype))) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        D_is_pattern = op_is_second || op_is_pair ;
        B_is_pattern = op_is_first  || op_is_pair ;
        ASSERT (GB_IMPLIES (!D_is_pattern,
            GB_Type_compatible (D->type, mult->xtype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->ytype))) ;
    }

    //--------------------------------------------------------------------------
    // C = D*B, row scale, via built-in binary operators
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_DxB(mult,xname) GB_DxB_ ## mult ## xname

        #define GB_BINOP_WORKER(mult,xname)                                  \
        {                                                                    \
            info = GB_DxB(mult,xname) (C, D, D_is_pattern, B, B_is_pattern,  \
                nthreads) ;                                                  \
            done = (info != GrB_NO_VALUE) ;                                  \
        }                                                                    \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Type_code xcode, ycode, zcode ;
        if (GB_binop_builtin (D->type, D_is_pattern, B->type, B_is_pattern,
            mult, flipxy, &opcode, &xcode, &ycode, &zcode))
        { 
            // C=D*B, rowscale with built-in operator
            #define GB_BINOP_IS_SEMIRING_MULTIPLIER
            #include "GB_binop_factory.c"
            #undef  GB_BINOP_IS_SEMIRING_MULTIPLIER
        }

    #endif

    //--------------------------------------------------------------------------
    // C = D*B, row scale, with typecasting or user-defined operator
    //--------------------------------------------------------------------------

    if (!done)
    {

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of D, B, and C
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (C, "(generic C=D*B rowscale) ") ;

        GxB_binary_function fmult = mult->function ;

        size_t csize = C->type->size ;
        size_t dsize = D_is_pattern ? 0 : D->type->size ;
        size_t bsize = B_is_pattern ? 0 : B->type->size ;

        size_t xsize = mult->xtype->size ;
        size_t ysize = mult->ytype->size ;

        // scalar workspace: because of typecasting, the x/y types need not
        // be the same as the size of the D and B types.
        // flipxy false: dii = (xtype) D(i,i) and bij = (ytype) B(i,j)
        // flipxy true:  dii = (ytype) D(i,i) and bij = (xtype) B(i,j)
        size_t dii_size = flipxy ? ysize : xsize ;
        size_t bij_size = flipxy ? xsize : ysize ;

        GB_void *GB_RESTRICT Cx = (GB_void *) C->x ;

        GB_cast_function cast_D, cast_B ;
        if (flipxy)
        { 
            // D is typecasted to y, and B is typecasted to x
            cast_D = D_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, D->type->code) ;
            cast_B = B_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, B->type->code) ;
        }
        else
        { 
            // D is typecasted to x, and B is typecasted to y
            cast_D = D_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, D->type->code) ;
            cast_B = B_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, B->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = D*B via function pointers, and typecasting
        //----------------------------------------------------------------------

        // dii = D(i,i), located in Dx [i]
        #define GB_GETA(dii,Dx,i)                                           \
            GB_void dii [GB_VLA(dii_size)] ;                                \
            if (!D_is_pattern) cast_D (dii, Dx +((i)*dsize), dsize) ;

        // bij = B(i,j), located in Bx [pB]
        #define GB_GETB(bij,Bx,pB)                                          \
            GB_void bij [GB_VLA(bij_size)] ;                                \
            if (!B_is_pattern) cast_B (bij, Bx +((pB)*bsize), bsize) ;

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
            #include "GB_AxB_rowscale_meta.c"
            #undef GB_BINOP
        }
        else
        { 
            #define GB_BINOP(z,x,y,i,j) fmult (z,x,y)
            #include "GB_AxB_rowscale_meta.c"
            #undef GB_BINOP
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "rowscale: C = D*B output", GB0) ;
    ASSERT (*Chandle == C) ;
    return (GrB_SUCCESS) ;
}


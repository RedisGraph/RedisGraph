//------------------------------------------------------------------------------
// GB_AxB_colscale: C = A*D where D is diagonal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_binop.h"
#include "GB_apply.h"
#include "GB_ek_slice.h"
#include "GB_stringify.h"
#ifndef GBCUDA_DEV
#include "GB_binop__include.h"
#endif
#include "GB_unused.h"

#define GB_FREE_WORKSPACE                   \
{                                           \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
}

#define GB_FREE_ALL                 \
{                                   \
    GB_FREE_WORKSPACE ;             \
    GB_phybix_free (C) ;            \
}

GrB_Info GB_AxB_colscale            // C = A*D, column scale with diagonal D
(
    GrB_Matrix C,                   // output matrix, static header
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
    ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;
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
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;

    GBURBLE ("(%s=%s*%s) ",
        GB_sparsity_char_matrix (A),    // C has the sparsity structure of A
        GB_sparsity_char_matrix (A),
        GB_sparsity_char_matrix (D)) ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Type ztype = mult->ztype ;
    ASSERT (ztype == semiring->add->op->ztype) ;
    GB_Opcode opcode = mult->opcode ;
    // GB_reduce_to_vector does not use GB_AxB_colscale:
    ASSERT (!(mult->binop_function == NULL &&
        (opcode == GB_FIRST_binop_code || opcode == GB_SECOND_binop_code))) ;

    //--------------------------------------------------------------------------
    // determine if C is iso (ignore the monoid since it isn't used)
    //--------------------------------------------------------------------------

    size_t zsize = ztype->size ;
    GB_void cscalar [GB_VLA(zsize)] ;
    bool C_iso = GB_iso_AxB (cscalar, A, D, A->vdim, semiring, flipxy, true) ;

    #ifdef GB_DEBUGIFY_DEFN
    GB_debugify_mxm (C_iso, GB_sparsity (A), ztype, NULL, false, false,
        semiring, flipxy, A, D) ;
    #endif

    //--------------------------------------------------------------------------
    // copy the pattern of A into C
    //--------------------------------------------------------------------------

    // allocate C->x but do not initialize it
    // set C->iso = C_iso   OK
    GB_OK (GB_dup_worker (&C, C_iso, A, false, ztype, Context)) ;
    GB_void *restrict Cx = (GB_void *) C->x ;

    //--------------------------------------------------------------------------
    // C = A*D, column scale, compute numerical values
    //--------------------------------------------------------------------------

    if (GB_OPCODE_IS_POSITIONAL (opcode))
    { 

        //----------------------------------------------------------------------
        // apply a positional operator: convert C=A*D to C=op(A)
        //----------------------------------------------------------------------

        // determine unary operator to compute C=A*D
        ASSERT (!flipxy) ;
        GrB_UnaryOp op = NULL ;
        if (ztype == GrB_INT64)
        {
            switch (opcode)
            {
                // first_op(A,D) becomes position_op(A)
                case GB_FIRSTI_binop_code   : op = GxB_POSITIONI_INT64 ; break;
                case GB_FIRSTJ_binop_code   : op = GxB_POSITIONJ_INT64 ; break;
                case GB_FIRSTI1_binop_code  : op = GxB_POSITIONI1_INT64; break;
                case GB_FIRSTJ1_binop_code  : op = GxB_POSITIONJ1_INT64; break;
                // second_op(A,D) becomes position_j(A)
                case GB_SECONDI_binop_code  : 
                case GB_SECONDJ_binop_code  : op = GxB_POSITIONJ_INT64 ; break;
                case GB_SECONDI1_binop_code : 
                case GB_SECONDJ1_binop_code : op = GxB_POSITIONJ1_INT64; break;
                default:  ;
            }
        }
        else
        {
            switch (opcode)
            {
                // first_op(A,D) becomes position_op(A)
                case GB_FIRSTI_binop_code   : op = GxB_POSITIONI_INT32 ; break;
                case GB_FIRSTJ_binop_code   : op = GxB_POSITIONJ_INT32 ; break;
                case GB_FIRSTI1_binop_code  : op = GxB_POSITIONI1_INT32; break;
                case GB_FIRSTJ1_binop_code  : op = GxB_POSITIONJ1_INT32; break;
                // second_op(A,D) becomes position_j(A)
                case GB_SECONDI_binop_code  : 
                case GB_SECONDJ_binop_code  : op = GxB_POSITIONJ_INT32 ; break;
                case GB_SECONDI1_binop_code : 
                case GB_SECONDJ1_binop_code : op = GxB_POSITIONJ1_INT32; break;
                default:  ;
            }
        }
        GB_OK (GB_apply_op (Cx, C->type, GB_NON_ISO,
            (GB_Operator) op,   // positional op
            NULL, false, false, A, Context)) ;
        ASSERT_MATRIX_OK (C, "colscale positional: C = A*D output", GB0) ;

    }
    else if (C_iso)
    { 

        //----------------------------------------------------------------------
        // C is iso; pattern already computed above
        //----------------------------------------------------------------------

        GBURBLE ("(iso colscale) ") ;
        memcpy (Cx, cscalar, zsize) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C is non iso
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // determine if the values are accessed
        //----------------------------------------------------------------------

        bool op_is_first  = (opcode == GB_FIRST_binop_code) ;
        bool op_is_second = (opcode == GB_SECOND_binop_code) ;
        bool op_is_pair   = (opcode == GB_PAIR_binop_code) ;
        bool A_is_pattern = false ;
        bool D_is_pattern = false ;
        ASSERT (!op_is_pair) ;

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

        //----------------------------------------------------------------------
        // determine the number of threads to use
        //----------------------------------------------------------------------

        GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

        //----------------------------------------------------------------------
        // slice the entries for each task
        //----------------------------------------------------------------------

        int A_nthreads, A_ntasks ;
        GB_SLICE_MATRIX (A, 32, chunk) ;

        bool done = false ;

        #ifndef GBCUDA_DEV

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_AxD(mult,xname) GB (_AxD_ ## mult ## xname)

            #define GB_BINOP_WORKER(mult,xname)                             \
            {                                                               \
                info = GB_AxD(mult,xname) (C, A, D,                         \
                    A_ek_slicing, A_ntasks, A_nthreads) ;                   \
                done = (info != GrB_NO_VALUE) ;                             \
            }                                                               \
            break ;

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            GB_Type_code xcode, ycode, zcode ;
            if (GB_binop_builtin (A->type, A_is_pattern, D->type, D_is_pattern,
                mult, flipxy, &opcode, &xcode, &ycode, &zcode))
            { 
                // C=A*D, colscale with built-in operator
                #define GB_BINOP_IS_SEMIRING_MULTIPLIER
                #define GB_NO_PAIR
                #include "GB_binop_factory.c"
                #undef  GB_BINOP_IS_SEMIRING_MULTIPLIER
            }

        #endif

        if (!done)
        {

            //------------------------------------------------------------------
            // C = A*D, column scale, with typecasting or user-defined operator
            //------------------------------------------------------------------

            //------------------------------------------------------------------
            // get operators, functions, workspace, contents of A, D, and C
            //------------------------------------------------------------------

            GB_BURBLE_MATRIX (C, "(generic C=A*D colscale) ") ;

            GxB_binary_function fmult = mult->binop_function ;

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

            //------------------------------------------------------------------
            // C = A*D via function pointers, and typecasting
            //------------------------------------------------------------------

            // aij = A(i,j), located in Ax [pA]
            #define GB_A_IS_PATTERN 0
            #define GB_GETA(aij,Ax,pA,A_iso)                            \
                GB_void aij [GB_VLA(aij_size)] ;                        \
                if (!A_is_pattern)                                      \
                {                                                       \
                    cast_A (aij, Ax +(A_iso ? 0:(pA)*asize), asize) ;   \
                }

            // dji = D(j,j), located in Dx [j]
            #define GB_B_IS_PATTERN 0
            #define GB_GETB(djj,Dx,j,D_iso)                             \
                GB_void djj [GB_VLA(djj_size)] ;                        \
                if (!D_is_pattern)                                      \
                {                                                       \
                    cast_D (djj, Dx +(D_iso ? 0:(j)*dsize), dsize) ;    \
                }

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
                #include "GB_AxB_colscale_template.c"
                #undef GB_BINOP
            }
            else
            { 
                #define GB_BINOP(z,x,y,i,j) fmult (z,x,y)
                #include "GB_AxB_colscale_template.c"
                #undef GB_BINOP
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "colscale: C = A*D output", GB0) ;
    GB_FREE_WORKSPACE ;
    return (GrB_SUCCESS) ;
}


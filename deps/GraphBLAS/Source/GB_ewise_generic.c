//------------------------------------------------------------------------------
// GB_ewise_generic: generic methods for eWiseMult and eWiseAdd
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_ewise_generic handles the generic case for ewise operations, when no
// built-in worker in the switch factory can handle this case.  This occurs
// for user-defined operators, when typecasting occurs, or for FIRST[IJ]* and
// SECOND[IJ]* positional operators.

// C is not iso, but A and/or B might be.

#include "GB_ewise.h"
#include "GB_emult.h"
#include "GB_binop.h"
#include "GB_unused.h"
#include "GB_ek_slice.h"

#undef  GB_FREE_ALL
#define GB_FREE_ALL             \
{                               \
    GB_phybix_free (C) ;        \
}

void GB_ewise_generic       // generic ewise
(
    // input/output:
    GrB_Matrix C,           // output matrix, static header
    // input:
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    // tasks from phase1a:
    const GB_task_struct *restrict TaskList,  // array of structs
    const int C_ntasks,                         // # of tasks
    const int C_nthreads,                       // # of threads to use
    // analysis from phase0:
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    const int C_sparsity,
    // from GB_emult_sparsity or GB_add_sparsity:
    const int ewise_method,
    // from GB_emult_04 and GB_emult_02:
    const int64_t *restrict Cp_kfirst,
    // to slice M, A, and/or B,
    const int64_t *M_ek_slicing, const int M_ntasks, const int M_nthreads,
    const int64_t *A_ek_slicing, const int A_ntasks, const int A_nthreads,
    const int64_t *B_ek_slicing, const int B_ntasks, const int B_nthreads,
    // original input:
    const GrB_Matrix M,             // optional mask, may be NULL
    const bool Mask_struct,         // if true, use the only structure of M
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for ewise generic", GB0) ;
    ASSERT_MATRIX_OK (A, "A for ewise generic", GB0) ;
    ASSERT_MATRIX_OK (B, "B for ewise generic", GB0) ;
    ASSERT_BINARYOP_OK (op, "op for ewise generic", GB0) ;
    ASSERT (!C->iso) ;

    //--------------------------------------------------------------------------
    // get C
    //--------------------------------------------------------------------------

    const GrB_Type ctype = C->type ;
    const GB_Type_code ccode = ctype->code ;

    //--------------------------------------------------------------------------
    // get the opcode and define the typecasting functions
    //--------------------------------------------------------------------------

    GB_Opcode opcode = op->opcode ;

    // the following booleans are all false if flipxy is true, since flipxy has
    // already been handled in the caller, in this case.
    const bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    const bool op_is_first  = (opcode == GB_FIRST_binop_code) ;
    const bool op_is_second = (opcode == GB_SECOND_binop_code) ;
    const bool op_is_pair   = (opcode == GB_PAIR_binop_code) ;
    const bool A_is_pattern = (op_is_second || op_is_pair || op_is_positional) ;
    const bool B_is_pattern = (op_is_first  || op_is_pair || op_is_positional) ;

    // if flipxy true use fop(y,x) else fop(x,y)
//  const bool flipxy = (ewise_method < 0) ;        TODO
    const bool flipxy = (ewise_method == GB_EMULT_METHOD3) ;

    const GxB_binary_function fop = op->binop_function ; // NULL if op positional
    const size_t csize = ctype->size ;
    const size_t asize = A->type->size ;
    const size_t bsize = B->type->size ;
    const GrB_Type xtype = flipxy ? op->ytype : op->xtype ;
    const GrB_Type ytype = flipxy ? op->xtype : op->ytype ;

    const size_t xsize = (A_is_pattern) ? 1 : xtype->size ;
    const size_t ysize = (B_is_pattern) ? 1 : ytype->size ;
    const size_t zsize = op->ztype->size ;

    const GB_cast_function cast_A_to_X =
        (A_is_pattern) ? NULL : GB_cast_factory (xtype->code, A->type->code) ;

    const GB_cast_function cast_B_to_Y = 
        (B_is_pattern) ? NULL : GB_cast_factory (ytype->code, B->type->code) ;

    const GB_cast_function cast_Z_to_C =
        GB_cast_factory (ccode, op->ztype->code) ;

    // aij = (xtype) A(i,j), located in Ax [pA]
    #define GB_GETA(aij,Ax,pA,A_iso)                                    \
        GB_void aij [GB_VLA(xsize)] ;                                   \
        if (cast_A_to_X != NULL)                                        \
        {                                                               \
            cast_A_to_X (aij, Ax +((A_iso) ? 0:(pA)*asize), asize) ;    \
        }

    // bij = (ytype) B(i,j), located in Bx [pB]
    #define GB_GETB(bij,Bx,pB,B_iso)                                    \
        GB_void bij [GB_VLA(ysize)] ;                                   \
        if (cast_B_to_Y != NULL)                                        \
        {                                                               \
            cast_B_to_Y (bij, Bx +((B_iso) ? 0:(pB)*bsize), bsize) ;    \
        }

    // address of Cx [p]
    #define GB_CX(p) Cx +((p)*csize)

    #define GB_ATYPE GB_void
    #define GB_BTYPE GB_void
    #define GB_CTYPE GB_void

    #define GB_PHASE_2_OF_2

    // loops cannot be vectorized
    #define GB_PRAGMA_SIMD_VECTORIZE ;

    // flipxy is handled in the definition of GB_BINOP, not in the tempate
    #define GB_FLIPPED 0

    //--------------------------------------------------------------------------
    // do the ewise operation
    //--------------------------------------------------------------------------

    if (op_is_positional)
    { 

        //----------------------------------------------------------------------
        // C(i,j) = positional_op (aij, bij)
        //----------------------------------------------------------------------

        const int64_t offset = GB_positional_offset (opcode, NULL) ;
        const bool index_is_i = 
            (opcode == GB_FIRSTI_binop_code  ) ||
            (opcode == GB_FIRSTI1_binop_code ) ||
            (opcode == GB_SECONDI_binop_code ) ||
            (opcode == GB_SECONDI1_binop_code) ;

        if (op->ztype == GrB_INT64)
        {
            #undef  GB_BINOP
            #define GB_BINOP(cij, aij, bij, i, j)                         \
                int64_t z = ((index_is_i) ? i : j) + offset ;             \
                cast_Z_to_C (cij, &z, csize) ;
            if (ewise_method == GB_EMULT_METHOD2 ||
                ewise_method == GB_EMULT_METHOD3)
            {
                #include "GB_emult_02_template.c"
            }
            else if (ewise_method == GB_EMULT_METHOD4)
            {
                #include "GB_emult_04_template.c"
            }
            else if (C_sparsity == GxB_BITMAP)
            {
                #include "GB_bitmap_emult_template.c"
            }
            else
            {
                #include "GB_emult_meta.c"
            }
        }
        else
        {
            #undef  GB_BINOP
            #define GB_BINOP(cij, aij, bij, i, j)                         \
                int32_t z = (int32_t) (((index_is_i) ? i : j) + offset) ; \
                cast_Z_to_C (cij, &z, csize) ;
            if (ewise_method == GB_EMULT_METHOD2 ||
                ewise_method == GB_EMULT_METHOD3)
            {
                #include "GB_emult_02_template.c"
            }
            else if (ewise_method == GB_EMULT_METHOD4)
            {
                #include "GB_emult_04_template.c"
            }
            else if (C_sparsity == GxB_BITMAP)
            {
                #include "GB_bitmap_emult_template.c"
            }
            else
            {
                #include "GB_emult_meta.c"
            }
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // standard binary operator
        //----------------------------------------------------------------------

        // C(i,j) = (ctype) (A(i,j) + B(i,j))
        if (ewise_method == GB_EMULT_METHOD2 ||
            ewise_method == GB_EMULT_METHOD3)
        { 
            // handle flipxy
            #undef  GB_BINOP
            #define GB_BINOP(cij, aij, bij, i, j)   \
                GB_void z [GB_VLA(zsize)] ;         \
                if (flipxy)                         \
                {                                   \
                    fop (z, bij, aij) ;             \
                }                                   \
                else                                \
                {                                   \
                    fop (z, aij, bij) ;             \
                }                                   \
                cast_Z_to_C (cij, z, csize) ;
            #include "GB_emult_02_template.c"
        }
        else if (ewise_method == GB_EMULT_METHOD4)
        { 
            #undef  GB_BINOP
            #define GB_BINOP(cij, aij, bij, i, j)   \
                GB_void z [GB_VLA(zsize)] ;         \
                fop (z, aij, bij) ;                 \
                cast_Z_to_C (cij, z, csize) ;
            #include "GB_emult_04_template.c"
        }
        else if (C_sparsity == GxB_BITMAP)
        { 
            #include "GB_bitmap_emult_template.c"
        }
        else
        { 
            #include "GB_emult_meta.c"
        }
    }

    ASSERT_MATRIX_OK (C, "C from ewise generic", GB0) ;
}


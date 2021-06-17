//------------------------------------------------------------------------------
// GB_add_phase2: C=A+B or C<M>=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_add_phase2 computes C=A+B, C<M>=A+B, or C<!M>A+B.  It is preceded first
// by GB_add_phase0, which computes the list of vectors of C to compute (Ch)
// and their location in A and B (C_to_[AB]).  Next, GB_add_phase1 counts the
// entries in each vector C(:,j) and computes Cp.

// GB_add_phase2 computes the pattern and values of each vector of C(:,j),
// entirely in parallel.

// C, M, A, and B can be standard sparse or hypersparse, as determined by
// GB_add_phase0.  The mask can be either: not present, or present and
// not complemented.  The complemented mask is handled in most cases,
// except when C, M, A, and B are all sparse or hypersparse.

// This function either frees Cp and Ch, or transplants then into C, as C->p
// and C->h.  Either way, the caller must not free them.

// op may be NULL.  In this case, the intersection of A and B must be empty.
// This is used by GB_Matrix_wait only, for merging the pending tuple matrix T
// into A.  In this case, C is always sparse or hypersparse, not bitmap or
// full.

#include "GB_add.h"
#include "GB_binop.h"
#include "GB_unused.h"
#include "GB_ek_slice.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"
#endif

#undef  GB_FREE_WORK
#define GB_FREE_WORK                                                    \
{                                                                       \
    GB_ek_slice_free (&pstart_Mslice, &kfirst_Mslice, &klast_Mslice) ;  \
    GB_ek_slice_free (&pstart_Aslice, &kfirst_Aslice, &klast_Aslice) ;  \
    GB_ek_slice_free (&pstart_Bslice, &kfirst_Bslice, &klast_Bslice) ;  \
}

#undef  GB_FREE_ALL
#define GB_FREE_ALL         \
{                           \
    GB_FREE_WORK ;          \
    GB_Matrix_free (&C) ;   \
}

GrB_Info GB_add_phase2      // C=A+B, C<M>=A+B, or C<!M>=A+B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_BinaryOp op,  // op to perform C = op (A,B), or NULL if no op
    // from phase1:
    const int64_t *GB_RESTRICT Cp,  // vector pointers for C
    const int64_t Cnvec_nonempty,   // # of non-empty vectors in C
    // tasks from phase1a:
    const GB_task_struct *GB_RESTRICT TaskList,    // array of structs
    const int C_ntasks,         // # of tasks
    const int C_nthreads,       // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const bool Ch_is_Mh,        // if true, then Ch == M->h
    const int C_sparsity,
    // original input:
    const GrB_Matrix M,         // optional mask, may be NULL
    const bool Mask_struct,     // if true, use the only structure of M
    const bool Mask_comp,       // if true, use !M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_BINARYOP_OK_OR_NULL (op, "op for add phase2", GB0) ;
    ASSERT_MATRIX_OK (A, "A for add phase2", GB0) ;
    ASSERT_MATRIX_OK (B, "B for add phase2", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for add phase2", GB0) ;
    ASSERT (A->vdim == B->vdim) ;

    ASSERT (!GB_JUMBLED (M)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_JUMBLED (B)) ;

    int64_t *pstart_Mslice = NULL, *kfirst_Mslice = NULL, *klast_Mslice = NULL ;
    int64_t *pstart_Aslice = NULL, *kfirst_Aslice = NULL, *klast_Aslice = NULL ;
    int64_t *pstart_Bslice = NULL, *kfirst_Bslice = NULL, *klast_Bslice = NULL ;

    //--------------------------------------------------------------------------
    // get the opcode
    //--------------------------------------------------------------------------

    bool C_is_hyper = (C_sparsity == GxB_HYPERSPARSE) ;
    bool C_is_sparse_or_hyper = (C_sparsity == GxB_SPARSE) || C_is_hyper ;
    ASSERT (C_is_sparse_or_hyper == (Cp != NULL)) ;
    ASSERT (C_is_hyper == (Ch != NULL)) ;

    GB_Opcode opcode = (op == NULL) ? GB_NOP_opcode : op->opcode ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    bool op_is_first  = (opcode == GB_FIRST_opcode) ;
    bool op_is_second = (opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = (opcode == GB_PAIR_opcode) ;

    if (op == NULL)
    { 
        // GB_Matrix_wait does no typecasting.  A and T have the same type when
        // computing A=A+T, and no operator is used since A and T have disjoint
        // nonzero patterns.  No mask is used.
        ASSERT (ctype == A->type) ;
        ASSERT (ctype == B->type) ;
        ASSERT (M == NULL) ;
        ASSERT (C_is_sparse_or_hyper) ;
    }
    else
    { 
        ASSERT (GB_Type_compatible (ctype, A->type)) ;
        ASSERT (GB_Type_compatible (ctype, B->type)) ;
        ASSERT (GB_Type_compatible (ctype, op->ztype)) ;
        ASSERT (GB_IMPLIES (!(op_is_second || op_is_pair || op_is_positional),
                GB_Type_compatible (A->type, op->xtype))) ;
        ASSERT (GB_IMPLIES (!(op_is_first  || op_is_pair || op_is_positional),
                GB_Type_compatible (B->type, op->ytype))) ;
    }

    //--------------------------------------------------------------------------
    // allocate the output matrix C: hypersparse, sparse, bitmap, or full
    //--------------------------------------------------------------------------

    // C is hypersparse if both A and B are (contrast with GrB_Matrix_emult),
    // or if M is present, not complemented, and hypersparse.
    // C acquires the same hyperatio as A.

    int64_t cnz = (C_is_sparse_or_hyper) ? Cp [Cnvec] : (A->vlen*A->vdim) ;
    (*Chandle) = NULL ;

    // allocate the result C (but do not allocate C->p or C->h)
    GrB_Matrix C = NULL ;
    GrB_Info info = GB_new_bix (&C, // any sparsity, new header
        ctype, A->vlen, A->vdim, GB_Ap_null, C_is_csc,
        C_sparsity, true, A->hyper_switch, Cnvec, cnz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory; caller must free C_to_M, C_to_A, C_to_B
        GB_FREE (Cp) ;
        GB_FREE (Ch) ;
        return (info) ;
    }

    // add Cp as the vector pointers for C, from GB_add_phase1
    if (C_is_sparse_or_hyper)
    {
        C->nvec_nonempty = Cnvec_nonempty ;
        C->p = (int64_t *) Cp ;
    }

    // add Ch as the hypersparse list for C, from GB_add_phase0
    if (C_is_hyper)
    { 
        C->h = (int64_t *) Ch ;
        C->nvec = Cnvec ;
    }

    // now Cp and Ch have been transplanted into C, so they must not be freed.
    C->magic = GB_MAGIC ;
    GB_Type_code ccode = ctype->code ;

    //--------------------------------------------------------------------------
    // check if the values of A and/or B are ignored
    //--------------------------------------------------------------------------

    // With C = ewiseadd (A,B), the union of A and B is used.
    // Entries in A but not B, and in B but not A, are copied to C without using
    // the operator, so entries in A and B are never ignored.
    // Contrast with ewisemult.

    // A is passed as x, and B as y, in z = op(x,y)
    bool A_is_pattern = false ;
    bool B_is_pattern = false ;

    //--------------------------------------------------------------------------
    // using a built-in binary operator (except for positional operators)
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_AaddB(mult,xname) GB_AaddB_ ## mult ## xname

        #define GB_BINOP_WORKER(mult,xname)                                 \
        {                                                                   \
            info = GB_AaddB(mult,xname) (C, C_sparsity,                     \
                M, Mask_struct, Mask_comp,                                  \
                A, B, Ch_is_Mh, C_to_M, C_to_A, C_to_B,                     \
                TaskList, C_ntasks, C_nthreads, Context) ;                  \
            done = (info != GrB_NO_VALUE) ;                                 \
        }                                                                   \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        GB_Type_code xcode, ycode, zcode ;
        if (!op_is_positional &&
            GB_binop_builtin (A->type, A_is_pattern, B->type, B_is_pattern,
            op, false, &opcode, &xcode, &ycode, &zcode) && ccode == zcode)
        { 
            #include "GB_binop_factory.c"
        }

        if (info == GrB_OUT_OF_MEMORY)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (info) ;
        }

    #endif

    //--------------------------------------------------------------------------
    // generic worker for positional ops, user-defined ops, and typecasting
    //--------------------------------------------------------------------------

    if (!done)
    {
        GB_BURBLE_MATRIX (C, "(generic add: %s) ",
            (op == NULL) ? "second" : op->name) ;

        GxB_binary_function fadd ;
        size_t csize, asize, bsize, xsize, ysize, zsize ;
        GB_cast_function cast_A_to_C, cast_B_to_C ;
        GB_cast_function cast_A_to_X, cast_B_to_Y, cast_Z_to_C ;

        if (op == NULL)
        { 
            // implicit GB_SECOND_[type] operator with no typecasting
            fadd = NULL ;               // the operator is not called
            csize = ctype->size ;
            asize = csize ;
            bsize = csize ;
            xsize = csize ;
            ysize = csize ;
            zsize = csize ;
            cast_A_to_X = GB_copy_user_user ;
            cast_B_to_Y = GB_copy_user_user ;
            cast_A_to_C = GB_copy_user_user ;
            cast_B_to_C = GB_copy_user_user ;
            cast_Z_to_C = GB_copy_user_user ;
        }
        else
        { 
            // normal case, C = A + B with optional typecasting
            fadd = op->function ;       // NULL if op is positional
            csize = ctype->size ;
            asize = A->type->size ;
            bsize = B->type->size ;

            if (op_is_second || op_is_pair || op_is_positional)
            { 
                // the op does not depend on the value of A(i,j)
                xsize = 1 ;
                cast_A_to_X = NULL ;
            }
            else
            { 
                xsize = op->xtype->size ;
                cast_A_to_X = GB_cast_factory (op->xtype->code, A->type->code) ;
            }

            if (op_is_first || op_is_pair || op_is_positional)
            { 
                // the op does not depend on the value of B(i,j)
                ysize = 1 ;
                cast_B_to_Y = NULL ;
            }
            else
            { 
                ysize = op->ytype->size ;
                cast_B_to_Y = GB_cast_factory (op->ytype->code, B->type->code) ;
            }

            zsize = op->ztype->size ;
            cast_A_to_C = GB_cast_factory (ccode, A->type->code) ;
            cast_B_to_C = GB_cast_factory (ccode, B->type->code) ;
            cast_Z_to_C = GB_cast_factory (ccode, op->ztype->code) ;
        }

        // C(i,j) = (ctype) A(i,j), located in Ax [pA]
        #define GB_COPY_A_TO_C(cij,Ax,pA)                                   \
            cast_A_to_C (cij, Ax +((pA)*asize), asize) ;

        // C(i,j) = (ctype) B(i,j), located in Bx [pB]
        #define GB_COPY_B_TO_C(cij,Bx,pB)                                   \
            cast_B_to_C (cij, Bx +((pB)*bsize), bsize) ;

        // aij = (xtype) A(i,j), located in Ax [pA]
        #define GB_GETA(aij,Ax,pA)                                          \
            GB_void aij [GB_VLA(xsize)] ;                                   \
            if (cast_A_to_X != NULL)                                        \
            {                                                               \
                cast_A_to_X (aij, Ax +((pA)*asize), asize) ;                \
            }

        // bij = (ytype) B(i,j), located in Bx [pB]
        #define GB_GETB(bij,Bx,pB)                                          \
            GB_void bij [GB_VLA(ysize)] ;                                   \
            if (cast_B_to_Y != NULL)                                        \
            {                                                               \
                cast_B_to_Y (bij, Bx +((pB)*bsize), bsize) ;                \
            }

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        #define GB_PHASE_2_OF_2

        // loops cannot be vectorized
        #define GB_PRAGMA_SIMD_VECTORIZE ;

        if (op_is_positional)
        { 

            //------------------------------------------------------------------
            // C(i,j) = positional_op (aij, bij)
            //------------------------------------------------------------------

            int64_t offset = GB_positional_offset (opcode) ;

            if (op->ztype == GrB_INT64)
            {
                switch (opcode)
                {
                    case GB_FIRSTI_opcode    : // z = first_i(A(i,j),y) == i
                    case GB_FIRSTI1_opcode   : // z = first_i1(A(i,j),y) == i+1
                    case GB_SECONDI_opcode   : // z = second_i(x,A(i,j)) == i
                    case GB_SECONDI1_opcode  : // z = second_i1(x,A(i,j)) == i+1
                        #undef  GB_BINOP
                        #define GB_BINOP(cij, aij, bij, i, j)   \
                            int64_t z = i + offset ;            \
                            cast_Z_to_C (cij, &z, csize) ;
                        #include "GB_add_template.c"
                        break ;
                    case GB_FIRSTJ_opcode    : // z = first_j(A(i,j),y) == j
                    case GB_FIRSTJ1_opcode   : // z = first_j1(A(i,j),y) == j+1
                    case GB_SECONDJ_opcode   : // z = second_j(x,A(i,j)) == j
                    case GB_SECONDJ1_opcode  : // z = second_j1(x,A(i,j)) == j+1
                        #undef  GB_BINOP
                        #define GB_BINOP(cij, aij, bij, i, j)   \
                            int64_t z = j + offset ;            \
                            cast_Z_to_C (cij, &z, csize) ;
                        #include "GB_add_template.c"
                        break ;
                    default: ;
                }
            }
            else
            {
                switch (opcode)
                {
                    case GB_FIRSTI_opcode    : // z = first_i(A(i,j),y) == i
                    case GB_FIRSTI1_opcode   : // z = first_i1(A(i,j),y) == i+1
                    case GB_SECONDI_opcode   : // z = second_i(x,A(i,j)) == i
                    case GB_SECONDI1_opcode  : // z = second_i1(x,A(i,j)) == i+1
                        #undef  GB_BINOP
                        #define GB_BINOP(cij, aij, bij, i, j)       \
                            int32_t z = (int32_t) (i + offset) ;    \
                            cast_Z_to_C (cij, &z, csize) ;
                        #include "GB_add_template.c"
                        break ;
                    case GB_FIRSTJ_opcode    : // z = first_j(A(i,j),y) == j
                    case GB_FIRSTJ1_opcode   : // z = first_j1(A(i,j),y) == j+1
                    case GB_SECONDJ_opcode   : // z = second_j(x,A(i,j)) == j
                    case GB_SECONDJ1_opcode  : // z = second_j1(x,A(i,j)) == j+1
                        #undef  GB_BINOP
                        #define GB_BINOP(cij, aij, bij, i, j)       \
                            int32_t z = (int32_t) (j + offset) ;    \
                            cast_Z_to_C (cij, &z, csize) ;
                        #include "GB_add_template.c"
                        break ;
                    default: ;
                }
            }

        }
        else
        { 

            //------------------------------------------------------------------
            // standard binary operator
            //------------------------------------------------------------------

            // C(i,j) = (ctype) (A(i,j) + B(i,j))
            // not used if op is null
            #undef  GB_BINOP
            #define GB_BINOP(cij, aij, bij, i, j)   \
                ASSERT (op != NULL) ;               \
                GB_void z [GB_VLA(zsize)] ;         \
                fadd (z, aij, bij) ;                \
                cast_Z_to_C (cij, z, csize) ;

            #include "GB_add_template.c"
        }
    }

    //--------------------------------------------------------------------------
    // remove empty vectors from C, if hypersparse
    //--------------------------------------------------------------------------

    GB_OK (GB_hypermatrix_prune (C, Context)) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    // caller must free C_to_M, C_to_A, and C_to_B, but not Cp or Ch
    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C output for add phase2", GB0) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}


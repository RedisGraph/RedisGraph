//------------------------------------------------------------------------------
// GB_bitmap_assign_noM_noaccum_whole:  assign to C bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// C<> = A             assign
// C<> = A             subassign

// C<repl> = A         assign
// C<repl> = A         subassign

// C<!> = A            assign
// C<!> = A            subassign

// C<!,repl> = A       assign
// C<!,repl> = A       subassign
//------------------------------------------------------------------------------

// C:           bitmap
// M:           none
// Mask_comp:   true or false
// Mask_struct: true or false (ignored)
// C_replace:   true or false
// accum:       not present
// A:           matrix (hyper, sparse, bitmap, or full), or scalar
// kind:        assign or subassign (same action)

// If M is not present and Mask_comp is true, then an empty mask is
// complemented.  This case is handled by GB_assign_prep:  if C_replace is
// true, the matrix is cleared by GB_clear, or no action is taken otherwise.
// In either case, this method is not called.  However, the "if (!Mask_comp)"
// test is left in below, for clarity.  Mask_comp will always be false here.

// For scalar assignment, C = x, this method just calls GB_dense_subassign_21,
// which handles any sparsity structure of C, including bitmap.

// For matrix assignment, C = A, if A is sparse or hyper and C may become
// sparse or hyper, then the assignement is done by GB_subassign_24.

#include "GB_bitmap_assign_methods.h"
#include "GB_dense.h"

#define GB_FREE_ALL ;

GrB_Info GB_bitmap_assign_noM_noaccum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
//  const GrB_Matrix M,         // mask matrix, not present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GBURBLE_BITMAP_ASSIGN ("bit6:whole", NULL, Mask_comp, NULL,
        GB_ALL, GB_ALL, GB_ASSIGN) ;
    ASSERT_MATRIX_OK (C, "C for bitmap assign: noM, noaccum", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (A, "A for bitmap assign: noM, noaccum", GB0) ;

    //--------------------------------------------------------------------------
    // do the assignment
    //--------------------------------------------------------------------------

    if (!Mask_comp)
    {

        //----------------------------------------------------------------------
        // C = A or C = scalar
        //----------------------------------------------------------------------

        if (A == NULL)
        { 

            //------------------------------------------------------------------
            // scalar assignment: C = scalar
            //------------------------------------------------------------------

            GB_OK (GB_dense_subassign_21 (C, scalar, scalar_type, Context)) ;

        }
        else
        { 

            //------------------------------------------------------------------
            // matrix assignment: C = A
            //------------------------------------------------------------------

            GB_GET_C_BITMAP ;           // C must be bitmap
            GB_GET_A_AND_SCALAR

            if (GB_IS_BITMAP (A) || GB_IS_FULL (A))
            { 

                //--------------------------------------------------------------
                // C = A where C is bitmap and A is bitmap or full
                //--------------------------------------------------------------

                // copy or typecast the values
                int nthreads = GB_nthreads (cnzmax, chunk, nthreads_max) ;
                GB_cast_array (Cx, C->type->code, (GB_void *) Ax,
                    A->type->code, Ab, A->type->size, cnzmax, nthreads) ;

                if (GB_IS_BITMAP (A))
                { 
                    // copy the bitmap
                    GB_memcpy (Cb, Ab, cnzmax, nthreads_max) ;
                    C->nvals = GB_NNZ (A) ;
                }
                else
                { 
                    // free the bitmap or set it to all ones
                    GB_bitmap_assign_to_full (C, nthreads_max) ;
                }

            }
            else
            {

                //--------------------------------------------------------------
                // C = A where C is bitmap and A is sparse or hyper
                //--------------------------------------------------------------

                int sparsity = GB_sparsity_control (C->sparsity, C->vdim) ;
                if ((GB_IS_SPARSE (A) && (sparsity & GxB_SPARSE)) ||
                    (GB_IS_HYPERSPARSE (A) && (sparsity & GxB_HYPERSPARSE)))
                { 
                    // C becomes sparse or hypersparse, the same as A
                    GB_OK (GB_subassign_24 (C, A, Context)) ;
                }
                else
                { 
                    // C remains bitmap: scatter A into the C bitmap
                    GB_memset (Cb, 0, cnzmax, nthreads_max) ;
                    cnvals = 0 ;
                    #define GB_AIJ_WORK(pC,pA)              \
                    {                                       \
                        /* Cx [pC] = Ax [pA] */             \
                        GB_ASSIGN_AIJ (pC, pA) ;            \
                        Cb [pC] = 1 ;                       \
                    }
                    #include "GB_bitmap_assign_A_whole_template.c"
                    C->nvals = GB_NNZ (A) ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "final C bitmap assign: noM, noaccum, whole", GB0) ;
    return (GrB_SUCCESS) ;
}


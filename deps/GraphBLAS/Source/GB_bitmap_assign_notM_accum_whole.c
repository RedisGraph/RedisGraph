//------------------------------------------------------------------------------
// GB_bitmap_assign_notM_accum_whole:  assign to C bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// C<!M> += A       assign
// C<!M> += A       subassign

// C<!M,repl> += A       assign
// C<!M,repl> += A       subassign
//------------------------------------------------------------------------------

// C:           bitmap
// M:           present, hypersparse or sparse (not bitmap or full)
// Mask_comp:   true
// Mask_struct: true or false
// C_replace:   true or false
// accum:       present
// A:           matrix (hyper, sparse, bitmap, or full), or scalar
// kind:        assign or subassign (same action)

#include "GB_bitmap_assign_methods.h"

#define GB_FREE_ALL \
    GB_ek_slice_free (&pstart_Mslice, &kfirst_Mslice, &klast_Mslice) ;

GrB_Info GB_bitmap_assign_notM_accum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Matrix M,         // mask matrix
//  const bool Mask_comp,       // true here, for !M only
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBURBLE_BITMAP_ASSIGN ("bit7:whole", M, true, accum,
        GB_ALL, GB_ALL, GB_ASSIGN) ;
    ASSERT (GB_IS_HYPERSPARSE (M) || GB_IS_SPARSE (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT_MATRIX_OK (C, "C for bitmap assign, !M, accum", GB0) ;
    ASSERT_MATRIX_OK (M, "M for bitmap assign, !M, accum", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (A, "A for bitmap assign, !M, accum", GB0) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GB_GET_C_BITMAP ;           // C must be bitmap
    GB_SLICE_M
    GB_GET_A_AND_SCALAR
    GB_GET_ACCUM_FOR_BITMAP

    //--------------------------------------------------------------------------
    // scatter M
    //--------------------------------------------------------------------------

    // Cb [pC] += 2 for each entry M(i,j) in the mask
    GB_bitmap_M_scatter_whole (C,
        M, Mask_struct, GB_BITMAP_M_SCATTER_PLUS_2,
        pstart_Mslice, kfirst_Mslice, klast_Mslice,
        M_nthreads, M_ntasks, Context) ;
    // the bitmap of C now contains:
    //  Cb (i,j) = 0:   cij not present, mij zero
    //  Cb (i,j) = 1:   cij present, mij zero
    //  Cb (i,j) = 2:   cij not present, mij 1
    //  Cb (i,j) = 3:   cij present, mij 1

    //--------------------------------------------------------------------------
    // do the assignment
    //--------------------------------------------------------------------------

    if (A == NULL)
    { 

        //----------------------------------------------------------------------
        // scalar assignment: C<!M, replace or !replace> += scalar
        //----------------------------------------------------------------------

        if (C_replace)
        { 

            //------------------------------------------------------------------
            // C<!M,replace> += scalar
            //------------------------------------------------------------------

            #undef  GB_CIJ_WORK
            #define GB_CIJ_WORK(pC)                                 \
            {                                                       \
                switch (Cb [pC])                                    \
                {                                                   \
                    case 0: /* C(i,j) not present, !M(i,j) = 1 */   \
                        /* Cx [pC] = scalar */                      \
                        GB_ASSIGN_SCALAR (pC) ;                     \
                        Cb [pC] = 1 ;                               \
                        task_cnvals++ ;                             \
                        break ;                                     \
                    case 1: /* C(i,j) present, !M(i,j) = 1 */       \
                        /* Cx [pC] += scalar */                     \
                        GB_ACCUM_SCALAR (pC) ;                      \
                        break ;                                     \
                    case 2: /* C(i,j) not present, !M(i,j) = 0 */   \
                        /* clear the mask from C */                 \
                        Cb [pC] = 0 ;                               \
                        break ;                                     \
                    case 3: /* C(i,j) present, !M(i,j) = 0 */       \
                        /* delete this entry */                     \
                        Cb [pC] = 0 ;                               \
                        task_cnvals-- ;                             \
                        break ;                                     \
                    default: ;                                      \
                }                                                   \
            }
            #include "GB_bitmap_assign_C_whole_template.c"

        }
        else
        {

            //------------------------------------------------------------------
            // C<!M> += scalar
            //------------------------------------------------------------------

            #undef  GB_CIJ_WORK
            #define GB_CIJ_WORK(pC)                                 \
            {                                                       \
                switch (Cb [pC])                                    \
                {                                                   \
                    case 0: /* C(i,j) not present, !M(i,j) = 1 */   \
                        /* Cx [pC] = scalar */                      \
                        GB_ASSIGN_SCALAR (pC) ;                     \
                        Cb [pC] = 1 ;                               \
                        task_cnvals++ ;                             \
                        break ;                                     \
                    case 1: /* C(i,j) present, !M(i,j) = 1 */       \
                        /* Cx [pC] += scalar */                     \
                        GB_ACCUM_SCALAR (pC) ;                      \
                        break ;                                     \
                    case 2: /* C(i,j) not present, !M(i,j) = 0 */   \
                        /* clear the mask from C */                 \
                        Cb [pC] = 0 ;                               \
                        break ;                                     \
                    case 3: /* C(i,j) present, !M(i,j) = 0 */       \
                        /* C(i,j) remains; clear the mask from C */ \
                        Cb [pC] = 1 ;                               \
                        break ;                                     \
                    default: ;                                      \
                }                                                   \
            }
            #include "GB_bitmap_assign_C_whole_template.c"
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // matrix assignment: C<!M, replace or !replace> += A
        //----------------------------------------------------------------------

        if (GB_IS_BITMAP (A) || GB_IS_FULL (A))
        {

            //------------------------------------------------------------------
            // C<!M, replace or !replace> += A where A is bitmap or full
            //------------------------------------------------------------------

            if (C_replace)
            {

                //--------------------------------------------------------------
                // C<!M, replace> += A where A is bitmap or full
                //--------------------------------------------------------------

                #undef  GB_CIJ_WORK
                #define GB_CIJ_WORK(pC)                                 \
                {                                                       \
                    switch (Cb [pC])                                    \
                    {                                                   \
                        case 0: /* C(i,j) not present, !M(i,j) = 1 */   \
                            if (GBB (Ab, pC))                           \
                            {                                           \
                                /* Cx [pC] = Ax [pC] */                 \
                                GB_ASSIGN_AIJ (pC, pC) ;                \
                                Cb [pC] = 1 ;                           \
                                task_cnvals++ ;                         \
                            }                                           \
                            break ;                                     \
                        case 1: /* C(i,j) present, !M(i,j) = 1 */       \
                            if (GBB (Ab, pC))                           \
                            {                                           \
                                /* Cx [pC] += Ax [pC] */                \
                                GB_ACCUM_AIJ (pC, pC) ;                 \
                            }                                           \
                            break ;                                     \
                        case 2: /* C(i,j) not present, !M(i,j) = 0 */   \
                            /* clear the mask from C */                 \
                            Cb [pC] = 0 ;                               \
                            break ;                                     \
                        case 3: /* C(i,j) present, !M(i,j) = 0 */       \
                            /* delete this entry */                     \
                            Cb [pC] = 0 ;                               \
                            task_cnvals-- ;                             \
                            break ;                                     \
                        default: ;                                      \
                    }                                                   \
                }
                #include "GB_bitmap_assign_C_whole_template.c"

            }
            else
            {

                //--------------------------------------------------------------
                // C<!M> += A where A is bitmap or full
                //--------------------------------------------------------------

                #undef  GB_CIJ_WORK
                #define GB_CIJ_WORK(pC)                                 \
                {                                                       \
                    switch (Cb [pC])                                    \
                    {                                                   \
                        case 0: /* C(i,j) not present, !M(i,j) = 1 */   \
                            if (GBB (Ab, pC))                           \
                            {                                           \
                                /* Cx [pC] = Ax [pC] */                 \
                                GB_ASSIGN_AIJ (pC, pC) ;                \
                                Cb [pC] = 1 ;                           \
                                task_cnvals++ ;                         \
                            }                                           \
                            break ;                                     \
                        case 1: /* C(i,j) present, !M(i,j) = 1 */       \
                            if (GBB (Ab, pC))                           \
                            {                                           \
                                /* Cx [pC] += Ax [pC] */                \
                                GB_ACCUM_AIJ (pC, pC) ;                 \
                            }                                           \
                            Cb [pC] = 1 ;                               \
                            break ;                                     \
                        case 2: /* C(i,j) not present, !M(i,j) = 0 */   \
                            /* clear the mask from C */                 \
                            Cb [pC] = 0 ;                               \
                            break ;                                     \
                        case 3: /* C(i,j) present, !M(i,j) = 0 */       \
                            /* keep the entry */                        \
                            Cb [pC] = 1 ;                               \
                            break ;                                     \
                        default: ;                                      \
                    }                                                   \
                }
                #include "GB_bitmap_assign_C_whole_template.c"
            }

        }
        else
        {

            //------------------------------------------------------------------
            // C<!M, replace or !replace> += A where A is sparse or hyper
            //------------------------------------------------------------------

            // assign or accumulate entries from A into C
            #undef  GB_AIJ_WORK
            #define GB_AIJ_WORK(pC,pA)          \
            {                                   \
                int8_t cb = Cb [pC] ;           \
                if (cb == 0)                    \
                {                               \
                    /* Cx [pC] = Ax [pA] */     \
                    GB_ASSIGN_AIJ (pC, pA) ;    \
                    Cb [pC] = 1 ;               \
                    task_cnvals++ ;             \
                }                               \
                else if (cb == 1)               \
                {                               \
                    /* Cx [pC] += Ax [pA] */    \
                    GB_ACCUM_AIJ (pC, pA) ;     \
                }                               \
            }
            #include "GB_bitmap_assign_A_whole_template.c"

            if (C_replace)
            { 
                // clear the mask and delete entries not assigned
                #undef  GB_MASK_WORK
                #define GB_MASK_WORK(pC)                \
                {                                       \
                    int8_t cb = Cb [pC] ;               \
                    Cb [pC] = 0 ;                       \
                    task_cnvals -= (cb == 3) ;          \
                }
                #include "GB_bitmap_assign_M_all_template.c"
            }
            else
            { 
                // clear the mask
                // Cb [pC] -= 2 for each entry M(i,j) in the mask
                GB_bitmap_M_scatter_whole (C,
                    M, Mask_struct, GB_BITMAP_M_SCATTER_MINUS_2,
                    pstart_Mslice, kfirst_Mslice, klast_Mslice,
                    M_nthreads, M_ntasks, Context) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    C->nvals = cnvals ;
    GB_FREE_ALL ;
    ASSERT_MATRIX_OK (C, "final C for bitmap assign, !M, accum, whole", GB0) ;
    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GB_dense_subassign_25: C(:,:)<M,s> = A; C empty, A dense, M structural
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Method 25: C(:,:)<M,s> = A ; C is empty, M structural, A dense

// M:           present
// Mask_comp:   false
// Mask_struct: true
// C_replace:   effectively false (not relevant since C is empty)
// accum:       NULL
// A:           matrix
// S:           none

// C and M are sparse or hypersparse.
// A can have any sparsity structure, even bitmap.  M may be jumbled.
// If so, C is constructed as jumbled.  C is reconstructed with the same
// structure as M and can have any sparsity structure on input.  The only
// constraint is nnz(C) is zero on input.  A must be dense with no pending
// work, or bitmap.

#include "GB_subassign_methods.h"
#include "GB_dense.h"
#ifndef GBCOMPACT
#include "GB_type__include.h"
#endif

#undef  GB_FREE_WORK
#define GB_FREE_WORK \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;

#undef  GB_FREE_ALL
#define GB_FREE_ALL GB_FREE_WORK

GrB_Info GB_dense_subassign_25
(
    GrB_Matrix C,
    // input:
    const GrB_Matrix M,
    const GrB_Matrix A,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_IS_BITMAP (M)) ; ASSERT (!GB_IS_FULL (M)) ;
    ASSERT (!GB_aliased (C, M)) ;   // NO ALIAS of C==M
    ASSERT (!GB_aliased (C, A)) ;   // NO ALIAS of C==A

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for subassign method_25", GB0) ;
    ASSERT (GB_NNZ (C) == 0) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ;

    ASSERT_MATRIX_OK (M, "M for subassign method_25", GB0) ;
    ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!GB_PENDING (M)) ;

    ASSERT_MATRIX_OK (A, "A for subassign method_25", GB0) ;
    ASSERT (GB_as_if_full (A) || GB_IS_BITMAP (A)) ;

    const GB_Type_code ccode = C->type->code ;

    //--------------------------------------------------------------------------
    // Method 25: C(:,:)<M> = A ; C is empty, A is dense, M is structural
    //--------------------------------------------------------------------------

    // Time: Optimal:  the method must iterate over all entries in M,
    // and the time is O(nnz(M)).  This is also the size of C.

    //--------------------------------------------------------------------------
    // Parallel: slice M into equal-sized chunks
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int64_t mnz = GB_NNZ_HELD (M) ;
    int nthreads = GB_nthreads (mnz + M->nvec, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (8 * nthreads) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, M, &ntasks))
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // allocate C and create its pattern
    //--------------------------------------------------------------------------

    // clear prior content and then create a copy of the pattern of M.  Keep
    // the same type and CSR/CSC for C.  Allocate the values of C but do not
    // initialize them.

    bool C_is_csc = C->is_csc ;
    GB_phbix_free (C) ;
    GB_OK (GB_dup2 (&C, M, false, C->type, Context)) ;
    C->is_csc = C_is_csc ;

    //--------------------------------------------------------------------------
    // C<M> = A for built-in types
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_Cdense_25(cname) GB_Cdense_25_ ## cname

        #define GB_WORKER(cname)                                              \
        {                                                                     \
            info = GB_Cdense_25(cname) (C, M, A,                              \
                kfirst_slice, klast_slice, pstart_slice, ntasks, nthreads) ;  \
            done = (info != GrB_NO_VALUE) ;                                   \
        }                                                                     \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        if (C->type == A->type && ccode < GB_UDT_code)
        { 
            // C<M> = A
            switch (ccode)
            {
                case GB_BOOL_code   : GB_WORKER (_bool  )
                case GB_INT8_code   : GB_WORKER (_int8  )
                case GB_INT16_code  : GB_WORKER (_int16 )
                case GB_INT32_code  : GB_WORKER (_int32 )
                case GB_INT64_code  : GB_WORKER (_int64 )
                case GB_UINT8_code  : GB_WORKER (_uint8 )
                case GB_UINT16_code : GB_WORKER (_uint16)
                case GB_UINT32_code : GB_WORKER (_uint32)
                case GB_UINT64_code : GB_WORKER (_uint64)
                case GB_FP32_code   : GB_WORKER (_fp32  )
                case GB_FP64_code   : GB_WORKER (_fp64  )
                case GB_FC32_code   : GB_WORKER (_fc32  )
                case GB_FC64_code   : GB_WORKER (_fc64  )
                default: ;
            }
        }

    #endif

    //--------------------------------------------------------------------------
    // C<M> = A for user-defined types, and typecasting
    //--------------------------------------------------------------------------

    if (!done)
    { 

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A and C
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (A, "(generic C(:,:)<M,struct>=A assign, method 25) ");

        const size_t csize = C->type->size ;
        const size_t asize = A->type->size ;
        const GB_Type_code acode = A->type->code ;
        GB_cast_function cast_A_to_C = GB_cast_factory (ccode, acode) ;

        // Cx [pC] = (ctype) Ax [pA]
        #define GB_COPY_A_TO_C(Cx,pC,Ax,pA) \
            cast_A_to_C (Cx + ((pC)*csize), Ax + ((pA)*asize), asize)

        #define GB_CTYPE GB_void
        #define GB_ATYPE GB_void

        // no vectorization
        #define GB_PRAGMA_SIMD_VECTORIZE ;

        #include "GB_dense_subassign_25_template.c"
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C output for subassign method_25", GB0) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    return (GrB_SUCCESS) ;
}


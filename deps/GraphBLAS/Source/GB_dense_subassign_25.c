//------------------------------------------------------------------------------
// GB_dense_subassign_25: C(:,:)<M,s> = A; C empty, A dense, M structural
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 25: C(:,:)<M,s> = A ; C is empty, M structure, A dense

// M:           present
// Mask_comp:   false
// Mask_struct: true
// C_replace:   effectively false (not relevant since C is empty)
// accum:       NULL
// A:           matrix
// S:           none

#include "GB_subassign_methods.h"
#include "GB_dense.h"
#ifndef GBCOMPACT
#include "GB_type__include.h"
#endif

#undef  GB_FREE_WORK
#define GB_FREE_WORK \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice, ntasks) ;

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
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for subassign method_25", GB0) ;
    ASSERT_MATRIX_OK (M, "M for subassign method_25", GB0) ;
    ASSERT_MATRIX_OK (A, "A for subassign method_25", GB0) ;
    ASSERT (GB_NNZ (C) == 0) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
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
    int64_t mnz = GB_NNZ (M) ;
    int nthreads = GB_nthreads (mnz + M->nvec, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (8 * nthreads) ;
    ntasks = GB_IMIN (ntasks, mnz) ;
    ntasks = GB_IMAX (ntasks, 1) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, M, ntasks))
    { 
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // allocate C and create its pattern
    //--------------------------------------------------------------------------

    // clear prior content and then create a copy of the pattern of M.  Keep
    // the same type and CSR/CSC for C.  Allocate the values of C but do not
    // initialize them.

    bool C_is_csc = C->is_csc ;
    GB_PHIX_FREE (C) ;
    GB_OK (GB_dup2 (&C, M, false, C->type, Context)) ;
    C->is_csc = C_is_csc ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    bool done = false ;

    #define GB_Cdense_25(xyname) GB_Cdense_25_ ## xyname

    #define GB_1TYPE_WORKER(xyname)                                         \
    {                                                                       \
        info = GB_Cdense_25(xyname) (C, M, A,                               \
            kfirst_slice, klast_slice, pstart_slice, ntasks, nthreads) ;    \
        done = (info != GrB_NO_VALUE) ;                                     \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        if (C->type == A->type && ccode < GB_UDT_code)
        { 
            // C<M> = A
            #include "GB_1type_factory.c"
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

        GB_BURBLE_MATRIX (A, "generic ") ;

        const size_t csize = C->type->size ;
        const size_t asize = A->type->size ;
        const size_t acode = A->type->code ;
        GB_cast_function cast_A_to_C = GB_cast_factory (ccode, acode) ;

        // Cx [pC] = (ctype) Ax [pA]
        #define GB_COPY_A_TO_C(Cx,pC,Ax,pA) \
            cast_A_to_C (Cx + ((pC)*csize), Ax + ((pA)*asize), asize)

        #define GB_CTYPE GB_void
        #define GB_ATYPE GB_void

        // no vectorization
        #define GB_PRAGMA_VECTORIZE

        #include "GB_dense_subassign_25_template.c"
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C output for subassign method_25", GB0) ;
    return (GrB_SUCCESS) ;
}


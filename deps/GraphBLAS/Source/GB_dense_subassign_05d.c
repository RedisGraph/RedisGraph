//------------------------------------------------------------------------------
// GB_dense_subassign_05d: C(:,:)<M> = scalar where C is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 05d: C(:,:)<M> = scalar ; no S, C is dense

// M:           present
// Mask_comp:   false
// C_replace:   false
// accum:       NULL
// A:           scalar
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

GrB_Info GB_dense_subassign_05d
(
    GrB_Matrix C,
    // input:
    const GrB_Matrix M,
    const bool Mask_struct,
    const GB_void *scalar,
    const GrB_Type atype,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (GB_is_dense (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT_MATRIX_OK (C, "C for subassign method_05d", GB0) ;
    const GB_Type_code ccode = C->type->code ;
    const size_t csize = C->type->size ;
    GB_GET_SCALAR ;

    //--------------------------------------------------------------------------
    // Method 05d: C(:,:)<M> = scalar ; no S; C is dense
    //--------------------------------------------------------------------------

    // Time: Optimal:  the method must iterate over all entries in M,
    // and the time is O(nnz(M)).

    //--------------------------------------------------------------------------
    // Parallel: slice M into equal-sized chunks
    //--------------------------------------------------------------------------

    int64_t mnz   = GB_NNZ (M) ;
    int64_t mnvec = M->nvec ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (mnz + mnvec, chunk, nthreads_max) ;
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
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    bool done = false ;

    #define GB_Cdense_05d(xyname) GB_Cdense_05d_ ## xyname

    #define GB_1TYPE_WORKER(xyname)                                         \
    {                                                                       \
        info = GB_Cdense_05d(xyname) (C, M, Mask_struct, cwork,             \
            kfirst_slice, klast_slice, pstart_slice, ntasks, nthreads) ;    \
        done = (info != GrB_NO_VALUE) ;                                     \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        // C<M> = x
        #include "GB_1type_factory.c"

    #endif

    //--------------------------------------------------------------------------
    // C<M> = x for user-defined types
    //--------------------------------------------------------------------------

    if (!done)
    { 

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A and C
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (M, "generic ") ;

        const size_t csize = C->type->size ;

        // Cx [p] = scalar
        #define GB_COPY_SCALAR_TO_C(p,x) \
            memcpy (Cx + ((p)*csize), x, csize)

        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_VECTORIZE

        #include "GB_dense_subassign_05d_template.c"
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C output for subassign method_05d", GB0) ;
    return (GrB_SUCCESS) ;
}


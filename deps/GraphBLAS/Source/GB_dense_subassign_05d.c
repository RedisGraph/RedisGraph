//------------------------------------------------------------------------------
// GB_dense_subassign_05d: C(:,:)<M> = scalar where C is as-if-full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Method 05d: C(:,:)<M> = scalar ; no S, C is dense

// M:           present
// Mask_comp:   false
// C_replace:   false
// accum:       NULL
// A:           scalar
// S:           none

// C can have any sparsity structure, but it must be entirely dense with
// all entries present.

#include "GB_subassign_methods.h"
#include "GB_dense.h"
#include "GB_unused.h"
#ifndef GBCUDA_DEV
#include "GB_type__include.h"
#endif

#undef  GB_FREE_WORKSPACE
#define GB_FREE_WORKSPACE                   \
{                                           \
    GB_WERK_POP (M_ek_slicing, int64_t) ;   \
}

#undef  GB_FREE_ALL
#define GB_FREE_ALL GB_FREE_WORKSPACE

GrB_Info GB_dense_subassign_05d
(
    GrB_Matrix C,
    // input:
    const GrB_Matrix M,
    const bool Mask_struct,
    const void *scalar,
    const GrB_Type atype,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_aliased (C, M)) ;   // NO ALIAS of C==M

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_WERK_DECLARE (M_ek_slicing, int64_t) ;

    ASSERT_MATRIX_OK (C, "C for subassign method_05d", GB0) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (GB_as_if_full (C)) ;

    ASSERT_MATRIX_OK (M, "M for subassign method_05d", GB0) ;
    ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!GB_PENDING (M)) ;

    GB_ENSURE_FULL (C) ;    // convert C to full, if sparsity control allows it
    if (C->iso)
    { 
        // work has already been done by GB_assign_prep
        return (GrB_SUCCESS) ;
    }

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

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    int M_ntasks, M_nthreads ;
    GB_SLICE_MATRIX (M, 8, chunk) ;

    //--------------------------------------------------------------------------
    // C<M> = x for built-in types
    //--------------------------------------------------------------------------

    bool done = false ;

    #ifndef GBCUDA_DEV

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_Cdense_05d(cname) GB (_Cdense_05d_ ## cname)

        #define GB_WORKER(cname)                                              \
        {                                                                     \
            info = GB_Cdense_05d(cname) (C, M, Mask_struct, cwork,            \
                M_ek_slicing, M_ntasks, M_nthreads) ;                         \
            done = (info != GrB_NO_VALUE) ;                                   \
        }                                                                     \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // C<M> = x
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

    #endif

    //--------------------------------------------------------------------------
    // C<M> = x for user-defined types
    //--------------------------------------------------------------------------

    if (!done)
    { 

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A and C
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (M, "(generic C(:,:)<M>=x assign) ") ;

        const size_t csize = C->type->size ;

        // Cx [p] = scalar
        #define GB_COPY_SCALAR_TO_C(p,x) \
            memcpy (Cx + ((p)*csize), x, csize)

        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_SIMD_VECTORIZE ;

        #include "GB_dense_subassign_05d_template.c"
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORKSPACE ;
    ASSERT_MATRIX_OK (C, "C output for subassign method_05d", GB0) ;
    return (GrB_SUCCESS) ;
}


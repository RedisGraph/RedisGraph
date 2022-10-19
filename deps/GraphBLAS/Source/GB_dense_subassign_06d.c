//------------------------------------------------------------------------------
// GB_dense_subassign_06d: C(:,:)<A> = A; C is full/bitmap, M and A are aliased
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Method 06d: C(:,:)<A> = A ; no S, C is dense, M and A are aliased

// M:           present
// Mask_comp:   false
// Mask_struct: true or false (both cases handled)
// C_replace:   false
// accum:       NULL
// A:           matrix, and aliased to M
// S:           none

// C must be bitmap or as-if-full.  No entries are deleted and thus no zombies
// are introduced into C.  C can be hypersparse, sparse, bitmap, or full, and
// its sparsity structure does not change.  If C is hypersparse, sparse, or
// full, then the pattern does not change (all entries are present, and this
// does not change), and these cases can all be treated the same (as if full).
// If C is bitmap, new entries can be inserted into the bitmap C->b.

// C and A can have any sparsity structure.

#include "GB_subassign_methods.h"
#include "GB_dense.h"
#ifndef GBCUDA_DEV
#include "GB_type__include.h"
#endif

#undef  GB_FREE_ALL
#define GB_FREE_ALL                         \
{                                           \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
}

GrB_Info GB_dense_subassign_06d
(
    GrB_Matrix C,
    // input:
    const GrB_Matrix A,
    bool Mask_struct,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;

    ASSERT_MATRIX_OK (C, "C for subassign method_06d", GB0) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (GB_IS_BITMAP (C) || GB_as_if_full (C)) ;
    ASSERT (!GB_aliased (C, A)) ;   // NO ALIAS of C==A

    ASSERT_MATRIX_OK (A, "A for subassign method_06d", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    const GB_Type_code ccode = C->type->code ;
    const bool C_is_bitmap = GB_IS_BITMAP (C) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_dense = GB_as_if_full (A) ;

    //--------------------------------------------------------------------------
    // Method 06d: C(:,:)<A> = A ; no S; C is dense, M and A are aliased
    //--------------------------------------------------------------------------

    // Time: Optimal:  the method must iterate over all entries in A,
    // and the time is O(nnz(A)).

    //--------------------------------------------------------------------------
    // Parallel: slice A into equal-sized chunks
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    int A_ntasks, A_nthreads ;
    if (A_is_bitmap || A_is_dense)
    { 
        // no need to construct tasks
        int64_t anz = GB_nnz_held (A) ;
        A_nthreads = GB_nthreads ((anz + A->nvec), 32*chunk, nthreads_max) ;
        A_ntasks = (A_nthreads == 1) ? 1 : (8 * A_nthreads) ;
    }
    else
    { 
        GB_SLICE_MATRIX (A, 8, 32*chunk) ;
    }

    //--------------------------------------------------------------------------
    // C<A> = A for built-in types
    //--------------------------------------------------------------------------

    if (C->iso)
    { 

        //----------------------------------------------------------------------
        // C is iso
        //----------------------------------------------------------------------

        // Since C is iso, A must be iso (or effectively iso), which is also
        // the mask M.  An iso mask matrix M is converted into a structural
        // mask by GB_get_mask, and thus Mask_struct must be true if C is iso.

        ASSERT (Mask_struct) ;
        #define GB_ISO_ASSIGN
        #include "GB_dense_subassign_06d_template.c"

    }
    else
    {

        //----------------------------------------------------------------------
        // C is non iso
        //----------------------------------------------------------------------

        bool done = false ;

        #ifndef GBCUDA_DEV

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_Cdense_06d(cname) GB (_Cdense_06d_ ## cname)

            #define GB_WORKER(cname)                                          \
            {                                                                 \
                info = GB_Cdense_06d(cname) (C, A, Mask_struct,               \
                    A_ek_slicing, A_ntasks, A_nthreads) ;                     \
                done = (info != GrB_NO_VALUE) ;                               \
            }                                                                 \
            break ;

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            if (C->type == A->type && ccode < GB_UDT_code)
            { 
                // C<A> = A
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

        //----------------------------------------------------------------------
        // C<A> = A for user-defined types, and typecasting
        //----------------------------------------------------------------------

        if (!done)
        { 

            //------------------------------------------------------------------
            // get operators, functions, workspace, contents of A and C
            //------------------------------------------------------------------

            GB_BURBLE_MATRIX (A, "(generic C(:,:)<Z>=Z assign) ") ;

            const size_t csize = C->type->size ;
            const size_t asize = A->type->size ;
            const GB_Type_code acode = A->type->code ;
            GB_cast_function cast_A_to_C = GB_cast_factory (ccode, acode) ;

            // Cx [p] = (ctype) Ax [pA]
            #define GB_COPY_A_TO_C(Cx,p,Ax,pA,A_iso) \
                cast_A_to_C (Cx + ((p)*csize), \
                    Ax + (A_iso ? 0:(pA)*asize), asize)

            #define GB_AX_MASK(Ax,pA,asize) \
                GB_mcast (Ax, pA, asize)

            #define GB_CTYPE GB_void
            #define GB_ATYPE GB_void

            // no vectorization
            #define GB_PRAGMA_SIMD_VECTORIZE ;
            #undef  GB_PRAGMA_SIMD_REDUCTION
            #define GB_PRAGMA_SIMD_REDUCTION(op,s) ;

            #include "GB_dense_subassign_06d_template.c"
        }
    }
    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    ASSERT_MATRIX_OK (C, "C output for subassign method_06d", GB0) ;
    return (GrB_SUCCESS) ;
}


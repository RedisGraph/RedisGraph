//------------------------------------------------------------------------------
// GB_subassign_24: make a deep copy of a sparse or dense matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = A, making a deep copy into an existing non-shallow matrix C, but
// possibly reusing parts of C if C is dense.  See also GB_dup.
// C can have any sparsity structure on input.  C takes on the same sparsity
// structure as A.

// Handles arbitrary typecasting.

// if C sparse and A dense/full, C is converted to full, ignoring
// C->sparsity.  C is conformed to its desired sparsity structure later.

// A can be jumbled, in which case C is also jumbled.
// A can have any sparsity structure (sparse, hyper, bitmap, or full).

#include "GB_dense.h"
#include "GB_Pending.h"
#define GB_FREE_ALL ;

GrB_Info GB_subassign_24    // C = A, copy A into an existing matrix C
(
    GrB_Matrix C,           // output matrix to modify
    const GrB_Matrix A,     // input matrix to copy
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_aliased (C, A)) ;   // NO ALIAS of C==A
    ASSERT (!GB_is_shallow (C)) ;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for GB_subassign_24", GB0) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (GB_PENDING_OK (C)) ;

    ASSERT_MATRIX_OK (A, "A for GB_subassign_24", GB0) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (A) ;

    C->jumbled = false ;        // prior contents of C are discarded

    // save the sparsity control of C
    int C_sparsity = C->sparsity ;
    float C_hyper_switch = C->hyper_switch ;
    float C_bitmap_switch = C->bitmap_switch ;

    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // C = A
    //--------------------------------------------------------------------------

    bool copy_dense_A_to_C =            // copy from dense A to dense C if:
        (
            GB_is_dense (C)             // both A and C are dense
            && GB_is_dense (A)
            && !(A->jumbled)            // A cannot be jumbled
            && C->vdim == A->vdim       // A and C have the same size
            && C->vlen == A->vlen
            && C->is_csc == A->is_csc   // A and C have the same format
            && C->x != NULL             // C->x exists
        ) ;

    if (copy_dense_A_to_C)
    { 

        //----------------------------------------------------------------------
        // discard the pattern of C
        //----------------------------------------------------------------------

        // make C full, if not full already
        C->nzombies = 0 ;                   // overwrite any zombies
        GB_Pending_free (&(C->Pending)) ;   // abandon all pending tuples
        GB_convert_any_to_full (C) ;        // ensure C is full

    }
    else
    { 

        //----------------------------------------------------------------------
        // copy the pattern from A to C
        //----------------------------------------------------------------------

        // clear prior content of C, but keep the CSR/CSC format and its type
        bool C_is_csc = C->is_csc ;
        GB_phbix_free (C) ;
        // copy the pattern, not the values
        GB_OK (GB_dup2 (&C, A, false, C->type, Context)) ;
        C->is_csc = C_is_csc ;      // do not change the CSR/CSC format of C
    }

    //-------------------------------------------------------------------------
    // copy the values from A to C, typecasting as needed
    //-------------------------------------------------------------------------

    if (C->type != A->type)
    { 
        GBURBLE ("(typecast) ") ;
    }

    int64_t anz = GB_NNZ_HELD (A) ;
    int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
    GB_cast_array (C->x, C->type->code, A->x, A->type->code,
        A->b, A->type->size, anz, nthreads) ;

    //-------------------------------------------------------------------------
    // restore the sparsity control of C
    //-------------------------------------------------------------------------

    C->sparsity = C_sparsity ;
    C->hyper_switch = C_hyper_switch ;
    C->bitmap_switch = C_bitmap_switch ;

    //-------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C result for GB_subassign_24", GB0) ;
    return (GrB_SUCCESS) ;
}


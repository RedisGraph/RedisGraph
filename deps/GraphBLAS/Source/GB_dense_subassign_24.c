//------------------------------------------------------------------------------
// GB_dense_subassign_24: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A, making a deep copy into an existing non-shallow matrix C, but
// possibly reusing parts of C if C is dense.  See also GB_dup.

#include "GB_dense.h"
#define GB_FREE_ALL ;

GrB_Info GB_dense_subassign_24      // C = A, copy A into an existing matrix C
(
    GrB_Matrix C,           // output matrix to modify
    const GrB_Matrix A,     // input matrix to copy
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C for C_dense_subassign_24", GB0) ;
    ASSERT_MATRIX_OK (A, "A for A_dense_subassign_24", GB0) ;
    ASSERT (GB_ZOMBIES_OK (A) && GB_PENDING_OK (A)) ;
    ASSERT (GB_ZOMBIES_OK (C) && GB_PENDING_OK (C)) ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    GB_WAIT (A) ;
    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // C = A
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;

    bool copy_dense_A_to_C =            // copy from dense A to dense C if:
        (
            GB_is_dense (C)             //      both A and C are dense
            && GB_is_dense (A)
            && !GB_ZOMBIES (C)          //      C has no pending work
            && !GB_PENDING (C)          // (FUTURE::: tolerate pending tuples)
//          && !GB_ZOMBIES (A)          //      A has no pending work
//          && !GB_PENDING (A)          //      (see GB_WAIT (A) above)
            && !(C->p_shallow)          //      C is not shallow
            && !(C->h_shallow)
            && !(C->i_shallow)
            && !(C->x_shallow)
            && !C->is_hyper             //      both A and C are standard
            && !A->is_hyper
            && C->vdim == A->vdim       //      A and C have the same size
            && C->vlen == A->vlen
            && C->is_csc == A->is_csc   //      A and C have the same format
            && C->p != NULL             //      C exists
            && C->i != NULL
            && C->x != NULL
            && C->h == NULL             //      C is standard
        ) ;

    if (copy_dense_A_to_C)
    { 

        //----------------------------------------------------------------------
        // copy the values from A to C; nothing else changes
        //----------------------------------------------------------------------

        GBBURBLE ("(dense copy) ") ;
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        GB_memcpy (C->x, A->x, anz * A->type->size, nthreads) ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // copy a sparse matrix from A to C
        //----------------------------------------------------------------------

        // clear all prior content of C, but keep the CSR/CSC format
        GBBURBLE ("(deep copy) ") ;
        GrB_Info info ;
        bool C_is_csc = C->is_csc ;
        GB_PHIX_FREE (C) ;
        GB_OK (GB_dup2 (&C, A, true, A->type, Context)) ;
        C->is_csc = C_is_csc ;      // do not change the CSR/CSC format of C
    }

    //-------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C result for C_dense_subassign_24", GB0) ;
    return (GrB_SUCCESS) ;
}


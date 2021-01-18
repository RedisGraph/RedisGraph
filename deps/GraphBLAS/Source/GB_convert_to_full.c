//------------------------------------------------------------------------------
// GB_convert_to_full: convert a matrix to full; deleting prior values
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_convert_to_full     // convert matrix to full; delete prior values
(
    GrB_Matrix A                // matrix to convert to full
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_void *Ax_new = NULL ;
    ASSERT_MATRIX_OK (A, "A converting to full", GB0) ;
    GBURBLE ("(to full) ") ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;
    ASSERT (GB_IS_FULL (A) || GB_IS_BITMAP (A) || GB_IS_SPARSE (A) ||
        GB_IS_HYPERSPARSE (A)) ;

    int64_t avdim = A->vdim ;
    int64_t avlen = A->vlen ;
    GrB_Index anzmax ;
    bool ok = GB_Index_multiply (&anzmax, avlen, avdim) ;
    if (!ok)
    { 
        // problem too large
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // free all prior content and allocate new space for A->x
    //--------------------------------------------------------------------------

    GB_phbix_free (A) ;

    Ax_new = GB_MALLOC (anzmax * A->type->size, GB_void) ;
    if (Ax_new == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // transplant the new content into A
    //--------------------------------------------------------------------------

    A->x = Ax_new ;
    A->plen = -1 ;
    A->nvec = avdim ;
    A->nvec_nonempty = (avlen == 0) ? 0 : avdim ;
    A->nzmax = GB_IMAX (anzmax, 1) ;
    A->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT (GB_IS_FULL (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    return (GrB_SUCCESS) ;
}


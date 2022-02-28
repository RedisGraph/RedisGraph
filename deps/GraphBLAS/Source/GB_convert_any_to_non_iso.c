//------------------------------------------------------------------------------
// GB_convert_any_to_non_iso: convert a matrix from iso to non-iso
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_convert_any_to_non_iso // convert iso matrix to non-iso
(
    GrB_Matrix A,           // input/output matrix
    bool initialize,        // if true, copy the iso value to all of A->x
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A to convert to non-iso", GB0) ;
    if (!A->iso)
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // get the iso entry of A
    //--------------------------------------------------------------------------

    size_t asize = A->type->size ;
    GB_void scalar [GB_VLA(asize)] ;
    if (initialize)
    { 
        memcpy (scalar, A->x, asize) ;
    }

    //--------------------------------------------------------------------------
    // ensure A->x is large enough, and not shallow
    //--------------------------------------------------------------------------

    int64_t anz = GB_nnz_held (A) ;
    anz = GB_IMAX (anz, 1) ;
    int64_t Ax_size_required = anz * asize ;

    if (A->x_size < Ax_size_required || A->x_shallow)
    {
        if (!A->x_shallow)
        { 
            // free the old space
            GB_FREE (&(A->x), A->x_size) ;
        }
        // allocate the new space
        A->x = GB_MALLOC (Ax_size_required, GB_void, &(A->x_size)) ; // x:OK
        A->x_shallow = false ;
        if (A->x == NULL)
        { 
            // out of memory
            GB_phbix_free (A) ;
            return (GrB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // copy the first entry into all of A->x
    //--------------------------------------------------------------------------

    if (initialize)
    { 
        GB_iso_expand (A->x, anz, scalar, asize, Context) ;
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return result
    //--------------------------------------------------------------------------

    A->iso = false ;        // OK: convert_any_to_non_iso
    ASSERT_MATRIX_OK (A, "A converted to non-iso", GB0) ;
    return (GrB_SUCCESS) ;
}


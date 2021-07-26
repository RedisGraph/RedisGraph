//------------------------------------------------------------------------------
// GB_convert_any_to_iso: convert a matrix from non-iso to iso
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If the matrix is not iso, it is changed to iso with the scalar value
// provided or with A->x [0] if the scalar is NULL on input.  The A->x array is
// optionally compacted to be exactly large enough to hold the single scalar.

#include "GB.h"

GrB_Info GB_convert_any_to_iso // convert non-iso matrix to iso
(
    GrB_Matrix A,           // input/output matrix
    GB_void *scalar,        // scalar value, of size A->type->size, or NULL
    bool compact,           // if true, also reduce the space for A->x
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A to convert to iso", GB0) ;

    //--------------------------------------------------------------------------
    // get the iso entry of A, or use the scalar provided on input
    //--------------------------------------------------------------------------

    size_t asize = A->type->size ;
    GB_void ascalar [GB_VLA(asize)] ;
    if (scalar == NULL)
    {
        if (A->iso)
        { 
            memcpy (ascalar, A->x, asize) ;
        }
        else
        { 
            memset (ascalar, 0, asize) ;
        }
    }

    //--------------------------------------------------------------------------
    // compact the matrix if required or requested
    //--------------------------------------------------------------------------

    if (A->x_size < asize || A->x_shallow || A->x == NULL
        || (compact && A->x_size != asize))
    {

        //----------------------------------------------------------------------
        // free the old A->x and allocate the new A->x
        //----------------------------------------------------------------------

        if (!A->x_shallow)
        { 
            // free the old space
            GB_FREE (&(A->x), A->x_size) ;
        }

        // allocate the new space
        A->x = GB_MALLOC (asize, GB_void, &(A->x_size)) ;
        A->x_shallow = false ;
        if (A->x == NULL)
        { 
            // out of memory
            GB_phbix_free (A) ;
            return (GrB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // copy the iso scalar into A->x [0]
    //--------------------------------------------------------------------------

    if (scalar == NULL)
    { 
        memcpy (A->x, ascalar, asize) ;
    }
    else
    { 
        memcpy (A->x, scalar, asize) ;
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return result
    //--------------------------------------------------------------------------

    A->iso = true ;     // OK: convert_any_to_iso
    ASSERT_MATRIX_OK (A, "A converted to iso", GB0) ;
    return (GrB_SUCCESS) ;
}


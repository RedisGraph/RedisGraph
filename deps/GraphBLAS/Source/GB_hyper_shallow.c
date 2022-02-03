//------------------------------------------------------------------------------
// GB_hyper_shallow: create a sparse shallow copy of a hypersparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The header of C itself is assumed to be statically allocated.  On input C
// must exist but the content of the C header is uninitialized.  No memory is
// allocated to construct C as the hyper_shallow version of A.  C is purely
// shallow.  If A is iso then so is C.

#include "GB.h"
#include "GB_convert.h"

GrB_Matrix GB_hyper_shallow         // return C
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix A              // input matrix, not modified.
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "hyper_shallow input", GB0) ;
    ASSERT (C != NULL) ;
    ASSERT (GB_IS_HYPERSPARSE (A)) ;

    //--------------------------------------------------------------------------
    // construct the shallow copy
    //--------------------------------------------------------------------------

    // copy the header
    memcpy (C, A, sizeof (struct GB_Matrix_opaque)) ;

    // flag the header of C as static
    C->static_header = true ;

    // remove the hyperlist
    C->h = NULL ;
    C->h_shallow = false ;

    // flag all content of C as shallow
    C->p_shallow = true ;
    C->i_shallow = true ;
    C->x_shallow = true ;

    // C reduces in dimension to the # of vectors in A
    C->vdim = C->nvec ;
    C->plen = C->nvec ;
    C->nvec_nonempty = C->nvec ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "hyper_shallow output", GB0) ;
    ASSERT (GB_IS_SPARSE (C)) ;
    return (C) ;
}


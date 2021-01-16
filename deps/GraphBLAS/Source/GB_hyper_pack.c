//------------------------------------------------------------------------------
// GB_hyper_pack: create a sparse shallow copy of a hypersparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_convert.h"

GrB_Matrix GB_hyper_pack            // return C
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix A              // input matrix
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "hyperpack input", GB0) ;
    ASSERT (C != NULL) ;
    ASSERT (GB_IS_HYPERSPARSE (A)) ;

    //--------------------------------------------------------------------------
    // construct the shallow copy
    //--------------------------------------------------------------------------

    // copy the header
    memcpy (C, A, sizeof (struct GB_Matrix_opaque)) ;

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

    ASSERT_MATRIX_OK (C, "hyperpack output", GB0) ;
    ASSERT (GB_IS_SPARSE (C)) ;
    return (C) ;
}


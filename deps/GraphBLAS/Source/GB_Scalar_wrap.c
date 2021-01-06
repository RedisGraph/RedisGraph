//------------------------------------------------------------------------------
// GB_Scalar_wrap: wrap a C scalar inside a GraphBLAS scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This method construct a shallow statically-defined scalar, with no memory
// allocations.  The scalar is full, with a single entry.

#include "GB.h"
#include "GB_scalar.h"

GxB_Scalar GB_Scalar_wrap   // create a new GxB_Scalar with one entry
(
    GxB_Scalar s,           // GxB_Scalar to create
    GrB_Type type,          // type of GxB_Scalar to create
    void *Sx                // becomes S->x, an array of size 1 * type->size
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (s != NULL) ;

    //--------------------------------------------------------------------------
    // create the GxB_Scalar
    //--------------------------------------------------------------------------

    s->magic = GB_MAGIC ;
    s->type = (type == NULL) ? GrB_BOOL : type ;
    s->hyper_switch  = GxB_NEVER_HYPER ;
    s->bitmap_switch = 0.5 ;
    s->sparsity = GxB_FULL ;
    s->plen = -1 ;
    s->vlen = 1 ;
    s->vdim = 1 ;
    s->nvec = 1 ;
    s->nvec_nonempty = 1 ;
    s->p = NULL ;
    s->h = NULL ;
    s->b = NULL ;
    s->i = NULL ;
    s->x = Sx ;
    s->nzmax = 1 ;
    s->Pending = NULL ;
    s->nzombies = 0 ;
    s->jumbled = false ;
    s->p_shallow = false ;
    s->h_shallow = false ;
    s->b_shallow = false ;
    s->i_shallow = false ;
    s->x_shallow = true ;
    s->is_csc = true ;
    // #include "GB_Scalar_wrap_mkl_template.c"

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (s) ;
}


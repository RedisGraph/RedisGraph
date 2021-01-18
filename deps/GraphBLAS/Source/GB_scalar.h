//------------------------------------------------------------------------------
// GB_scalar.h: definitions for GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_SCALAR_H
#define GB_SCALAR_H

GxB_Scalar GB_Scalar_wrap   // create a new GxB_Scalar with one entry
(
    GxB_Scalar s,           // GxB_Scalar to create
    GrB_Type type,          // type of GxB_Scalar to create
    void *Sx                // becomes S->x, an array of size 1 * type->size
) ;

// stype can be NULL if op is positional

// wrap a bare scalar inside a statically-allocated GxB_Scalar
#define GB_SCALAR_WRAP(scalar,prefix,T,ampersand,bare,stype)                \
    struct GB_Scalar_opaque scalar ## _struct ;                             \
    size_t ssize = (stype == NULL) ? 1 : (stype->size) ;                    \
    GB_void Sx [GB_VLA (ssize)] ;                                           \
    GxB_Scalar scalar = GB_Scalar_wrap (& scalar ## _struct, stype, Sx) ;   \
    memcpy (Sx, ampersand bare, ssize) ;

#endif


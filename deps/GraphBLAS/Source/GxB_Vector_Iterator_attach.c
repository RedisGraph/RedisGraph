//------------------------------------------------------------------------------
// GxB_Vector_Iterator_attach: attach an iterator to vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Vector_Iterator_attach
(
    // input/output:
    GxB_Iterator iterator,      // iterator to attach to the vector v
    // input
    GrB_Vector v,               // vector to attach
    GrB_Descriptor desc
)
{
    return (GB_Iterator_attach (iterator, (GrB_Matrix) v, GxB_NO_FORMAT,
        desc)) ;
}


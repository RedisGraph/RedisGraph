//------------------------------------------------------------------------------
// GB_sparsity_control: ensure the sparsity control is in the proper range
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

int GB_sparsity_control     // revised sparsity control
(
    int sparsity,           // sparsity control
    int64_t vdim            // A->vdim, or -1 to ignore this condition
)
{ 

    //--------------------------------------------------------------------------
    // ensure the sparsity control is in range 1 to 15
    //--------------------------------------------------------------------------

    sparsity = sparsity & GxB_ANY_SPARSITY ;
    if (sparsity == GxB_DEFAULT)
    { 
        // if zero, set to auto sparsity
        sparsity = GxB_AUTO_SPARSITY ;
    }

    //--------------------------------------------------------------------------
    // ensure vectors and scalars cannot become hypersparse
    //--------------------------------------------------------------------------

    if ((vdim == 0 || vdim == 1) && (sparsity & GxB_HYPERSPARSE))
    { 
        // a GxB_Scalar, GrB_Vector, or a GrB_Matrix with a single vector,
        // cannot be converted to hypersparse.  If the sparsity control
        // allows for the hypersparse case, disable it and enable the
        // sparse case instead.
        sparsity = sparsity & (~GxB_HYPERSPARSE) ;
        sparsity = sparsity | GxB_SPARSE ;
    }

    //--------------------------------------------------------------------------
    // return revised sparsity control
    //--------------------------------------------------------------------------

    return (sparsity) ;
}


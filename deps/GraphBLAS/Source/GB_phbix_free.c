//------------------------------------------------------------------------------
// GB_phbix_free: free all content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Frees all allocatable content of a matrix, except for the header itself.
// A->magic becomes GB_MAGIC2.  If this matrix is given to a user-callable
// GraphBLAS function, it will generate a GrB_INVALID_OBJECT error.

#include "GB.h"

void GB_phbix_free              // free all content of a matrix
(
    GrB_Matrix A                // handle of matrix with content to free
)
{ 

    GB_ph_free (A) ;
    GB_bix_free (A) ;
}


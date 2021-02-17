//------------------------------------------------------------------------------
// GB_Matrix_free: free a GrB_Matrix or GrB_Vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Free all the content of a matrix.  After GB_Matrix_free (&A), A is set to
// NULL.

#include "GB.h"

void GB_Matrix_free             // free a matrix
(
    GrB_Matrix *matrix          // handle of matrix to free
)
{

    if (matrix != NULL)
    {
        GrB_Matrix A = *matrix ;
        if (A != NULL && (A->magic == GB_MAGIC || A->magic == GB_MAGIC2))
        { 
            // free all content of A
            GB_phbix_free (A) ;
            // #include "GB_Matrix_free_mkl_template.c
            // free the error logger string
            GB_FREE (A->logger) ;
            // free the header of A itself
            A->magic = GB_FREED ;      // to help detect dangling pointers
            GB_FREE (*matrix) ;
        }
        (*matrix) = NULL ;
    }
}


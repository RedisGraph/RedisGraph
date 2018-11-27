//------------------------------------------------------------------------------
// GB_free: free a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// free all the content of a matrix.  After GB_free (&A), A is set to NULL.

#include "GB.h"

GrB_Info GB_free                // free a matrix
(
    GrB_Matrix *matrix          // handle of matrix to free
)
{

    if (matrix != NULL)
    {
        GrB_Matrix A = *matrix ;
        if (A != NULL && (A->magic == GB_MAGIC || A->magic == GB_MAGIC2))
        { 
            // free all content of A, including the Sauna
            GB_CONTENT_FREE (A) ;
            // free the header of A itself
            A->magic = GB_FREED ;      // to help detect dangling pointers
            GB_FREE_MEMORY (*matrix, 1, sizeof (struct GB_Matrix_opaque)) ;
        }
        (*matrix) = NULL ;
    }

    return (GrB_SUCCESS) ;
}


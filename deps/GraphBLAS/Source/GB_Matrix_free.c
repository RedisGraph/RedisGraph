//------------------------------------------------------------------------------
// GB_Matrix_free: free a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// free all the content of a matrix.  After GB_Matrix_free (&A), A is set
// to NULL.  This function is not user-callable; use GrB_Matrix_free instead.

#include "GB.h"

void GB_Matrix_free             // free a matrix
(
    GrB_Matrix *matrix          // handle of matrix to free
)
{

    if (matrix != NULL)
    {
        GrB_Matrix A = *matrix ;
        if (A != NULL && (A->magic == MAGIC || A->magic == MAGIC2))
        {
            A->magic = FREED ;      // to help detect dangling pointers
            if (!A->p_shallow)
            {
                GB_FREE_MEMORY (A->p, A->ncols+1, sizeof (int64_t)) ;
            }
            A->p = NULL ;
            GB_Matrix_ixfree (A) ;
            GB_FREE_MEMORY (*matrix, 1, sizeof (GB_Matrix_opaque)) ;
        }
        (*matrix) = NULL ;
    }
}


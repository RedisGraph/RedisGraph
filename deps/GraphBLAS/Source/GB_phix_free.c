//------------------------------------------------------------------------------
// GB_phix_free: free all content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Frees all allocatable content of a matrix, except for the header itself.
// A->magic becomes GB_MAGIC2.  If this matrix is given to a user-callable
// GraphBLAS function, it will generate a GrB_INVALID_OBJECT error.

#include "GB.h"

GrB_Info GB_phix_free           // free all content of a matrix
(
    GrB_Matrix A,               // handle of matrix with content to free
    bool free_sauna             // if true, also free the Sauna
)
{ 

    GB_ph_free (A) ;
    if (free_sauna && A != NULL)
    {
        GB_Sauna_free (&(A->Sauna)) ;
    }
    GB_IX_FREE (A) ;
    return (GrB_SUCCESS) ;
}


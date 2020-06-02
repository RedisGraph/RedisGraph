//------------------------------------------------------------------------------
// gb_is_shallow: determine if a GrB_matrix has any shallow components
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

bool gb_is_shallow              // true if any component of A is shallow
(
    GrB_Matrix A                // GrB_Matrix to query
)
{

    if (A == NULL)
    {
        // a NULL pointer is not shallow
        return (false) ;
    }
    else
    { 
        // check if any component of A is shallow
        return (A->p_shallow || A->h_shallow || A->i_shallow || A->x_shallow) ;
    }
}


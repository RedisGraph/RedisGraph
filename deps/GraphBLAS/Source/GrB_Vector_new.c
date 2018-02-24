//------------------------------------------------------------------------------
// GrB_Vector_new: create a new vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The new vector is n-by-1, with no entries in it.
// A->p is size 2 and all zero.  Contents A->x and A->i are NULL.
// If this method fails, *v is set to NULL.

#include "GB.h"

GrB_Info GrB_Vector_new     // create a new vector with no entries
(
    GrB_Vector *v,          // handle of vector to create
    const GrB_Type type,    // type of vector to create
    const GrB_Index n       // dimension is n-by-1
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Vector_new (&v, type, n)") ;
    RETURN_IF_NULL (v) ;
    (*v) = NULL ;
    RETURN_IF_NULL_OR_UNINITIALIZED (type) ;

    if (n > GB_INDEX_MAX)
    {
        // problem too large
        return (ERROR (GrB_INVALID_VALUE, (LOG,
            "problem too large: n "GBu" exceeds "GBu, n, GB_INDEX_MAX))) ;
    }

    //--------------------------------------------------------------------------
    // create the vector
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_NEW ((GrB_Matrix *) v, type, n, 1, true, false) ;
    return (info) ;
}


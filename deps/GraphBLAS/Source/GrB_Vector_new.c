//------------------------------------------------------------------------------
// GrB_Vector_new: create a new vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The new vector is n-by-1, with no entries in it.
// A->p is size 2 and all zero.  Contents A->x and A->i are NULL.
// If this method fails, *v is set to NULL.  Vectors are not hypersparse,
// so format is standard CSC, and A->h is NULL.

#include "GB.h"

GrB_Info GrB_Vector_new     // create a new vector with no entries
(
    GrB_Vector *v,          // handle of vector to create
    GrB_Type type,          // type of vector to create
    GrB_Index n             // dimension is n-by-1
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Vector_new (&v, type, n)") ;
    GB_RETURN_IF_NULL (v) ;
    (*v) = NULL ;
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;

    if (n > GB_INDEX_MAX)
    { 
        // problem too large
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "problem too large: n "GBu" exceeds "GBu, n, GB_INDEX_MAX))) ;
    }

    //--------------------------------------------------------------------------
    // create the vector
    //--------------------------------------------------------------------------

    GrB_Info info ;
    int64_t vlen = (int64_t) n ;

    // v is always non-hypersparse, but use the auto rule so that
    // v->hyper_ratio is assigned from the global option.  This way, if the
    // vector is ever typecast into a matrix, and used in a matrix computation,
    // the hyper_ratio will propagate to the result matrix.  A vector will not
    // use its hyper_ratio, since vdim == 1 ensures that v always remains
    // non-hypersparse.

    // *v == NULL ;                 // allocate a new header for v
    GB_NEW ((GrB_Matrix *) v, type, vlen, 1, GB_Ap_calloc, true,
        GB_AUTO_HYPER, GB_HYPER_DEFAULT, 1, Context) ;
    ASSERT (GB_IMPLIES (info == GrB_SUCCESS, GB_VECTOR_OK (*v))) ;
    return (info) ;
}


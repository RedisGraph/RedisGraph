//------------------------------------------------------------------------------
// GxB_Scalar_new: create a new GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The new GxB_Scalar has no entry.  Internally, it is identical to a
// GrB_Vector of length 1.  If this method fails, *s is set to NULL.

#include "GB.h"

GrB_Info GxB_Scalar_new     // create a new GxB_Scalar with no entries
(
    GxB_Scalar *s,          // handle of GxB_Scalar to create
    GrB_Type type           // type of GxB_Scalar to create
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Scalar_new (&s, type)") ;
    GB_RETURN_IF_NULL (s) ;
    (*s) = NULL ;
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;

    //--------------------------------------------------------------------------
    // create the GxB_Scalar
    //--------------------------------------------------------------------------

    GrB_Info info ;

    // *s == NULL ;                 // allocate a new header for s
    GB_NEW ((GrB_Matrix *) s, type, 1, 1, GB_Ap_calloc, true,
        GB_AUTO_HYPER, GB_HYPER_DEFAULT, 1, Context) ;
    ASSERT (GB_IMPLIES (info == GrB_SUCCESS, GB_SCALAR_OK (*s))) ;
    return (info) ;
}


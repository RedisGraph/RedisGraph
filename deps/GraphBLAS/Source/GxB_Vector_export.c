//------------------------------------------------------------------------------
// GxB_Vector_export: export a vector in CSR/CSC format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The indices are returned in sorted order.

#include "GB_export.h"

GrB_Info GxB_Vector_export  // export and free a vector
(
    GrB_Vector *v,          // handle of vector to export and free
    GrB_Type *type,         // type of vector exported
    GrB_Index *n,           // length of the vector
    GrB_Index *nvals,       // number of entries in the vector
    // CSR/CSC format:
    GrB_Index **vi,         // indices, size nvals (in sorted order)
    void      **vx,         // values, size nvals
    const GrB_Descriptor desc       // currently unused
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Vector_export (&v, &type, &n, &nvals, &vi, &vx, desc)") ;
    GB_BURBLE_START ("GxB_Vector_export") ;
    GB_RETURN_IF_NULL (v) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*v) ;
    ASSERT_VECTOR_OK (*v, "v to export", GB0) ;

    // finish any pending work
    GB_WAIT (*v) ;

    // check these after forcing completion
    GB_RETURN_IF_NULL (type) ;
    GB_RETURN_IF_NULL (n) ;
    GB_RETURN_IF_NULL (nvals) ;
    GB_RETURN_IF_NULL (vi) ;
    GB_RETURN_IF_NULL (vx) ;

    //--------------------------------------------------------------------------
    // export the vector
    //--------------------------------------------------------------------------

    // export basic attributes
    (*type) = (*v)->type ;
    (*n) = (*v)->vlen ;
    (*nvals) = GB_NNZ (*v) ;

    // export the content and remove it from v
    if ((*nvals) > 0)
    { 
        (*vi) = (GrB_Index *) (*v)->i ;
        (*vx) = (*v)->x ;
        (*v)->i = NULL ;
        (*v)->x = NULL ;
    }
    else
    { 
        (*vi) = NULL ;
        (*vx) = NULL ;
    }

    //--------------------------------------------------------------------------
    // export is successful
    //--------------------------------------------------------------------------

    // free the vector header; do not free the exported content of the vector,
    // which has already been removed above.
    GB_VECTOR_FREE (v) ;
    ASSERT (*v == NULL) ;
    GB_BURBLE_END ;
    return (GrB_SUCCESS) ;
}


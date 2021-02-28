//------------------------------------------------------------------------------
// GxB_Vector_import: import a vector in CSR/CSC format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The indices must appear in sorted order.

#include "GB_export.h"

GrB_Info GxB_Vector_import  // import a vector in CSC format
(
    GrB_Vector *v,          // handle of vector to create
    GrB_Type type,          // type of vector to create
    GrB_Index n,            // vector length
    GrB_Index nvals,        // number of entries in the vector
    // CSR/CSC format:
    GrB_Index **vi,         // indices, size nvals (in sorted order)
    void      **vx,         // values, size nvals
    const GrB_Descriptor desc       // currently unused
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Vector_import (&v, type, n, nvals, &vi, &vx, desc)") ;
    GB_BURBLE_START ("GxB_Vector_import") ;
    GB_RETURN_IF_NULL (v) ;
    (*v) = NULL ;
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;

    if (n > GB_INDEX_MAX)
    { 
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "problem too large: n "GBu" exceeds "GBu,
            n, GB_INDEX_MAX))) ;
    }
    if (nvals > GB_INDEX_MAX)
    { 
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "problem too large: nvals "GBu" exceeds "GBu,
            nvals, GB_INDEX_MAX))) ;
    }

    if (nvals > 0)
    { 
        GB_RETURN_IF_NULL (vi) ;
        GB_RETURN_IF_NULL (vx) ;
    }

    //--------------------------------------------------------------------------
    // import the vector
    //--------------------------------------------------------------------------

    GrB_Info info ;

    // allocate the header of the vector; allocate v->p of size 2 and clear it
    GB_NEW ((GrB_Matrix *) v, type, (int64_t) n, 1, GB_Ap_calloc, true,
        GB_AUTO_HYPER, GB_HYPER_DEFAULT, 1, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory for vector header (size O(1))
        ASSERT (*v == NULL) ;
        return (info) ;
    }

    // transplant the user's content into the vector
    (*v)->nzmax = nvals ;
    (*v)->p [1] = nvals ;

    if (nvals == 0)
    { 
        // free the user input vi and vx arrays, if they exist
        if (vi != NULL) GB_FREE_MEMORY (*vi, nvals, sizeof (GrB_Index)) ;
        if (vx != NULL) GB_FREE_MEMORY (*vx, nvals, type->size) ;
    }
    else
    { 
        // transplant vi and vx into the vector
        (*v)->i = (int64_t *) (*vi) ;
        (*v)->x = (*vx) ;
        (*vi) = NULL ;
        (*vx) = NULL ;
        (*v)->nvec_nonempty = 1 ;
    }

    //--------------------------------------------------------------------------
    // import is successful
    //--------------------------------------------------------------------------

    ASSERT (*vi == NULL) ;
    ASSERT (*vx == NULL) ;
    ASSERT_VECTOR_OK (*v, "v imported", GB0) ;
    GB_BURBLE_END ;
    return (GrB_SUCCESS) ;
}


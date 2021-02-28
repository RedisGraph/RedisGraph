//------------------------------------------------------------------------------
// GxB_Matrix_export_CSR: export a matrix in CSR format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_export.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_export_CSR  // export and free a CSR matrix
(
    GrB_Matrix *A,          // handle of matrix to export and free
    GrB_Type *type,         // type of matrix exported
    GrB_Index *nrows,       // matrix dimension is nrows-by-ncols
    GrB_Index *ncols,
    GrB_Index *nvals,       // number of entries in the matrix
    // CSR format:
    int64_t *nonempty,      // number of rows with at least one entry
    GrB_Index **Ap,         // row "pointers", size nrows+1
    GrB_Index **Aj,         // column indices, size nvals
    void      **Ax,         // values, size nvals
    const GrB_Descriptor desc       // descriptor for # of threads to use
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Matrix_export_CSR (&A, &type, &nrows, &ncols, &nvals,"
        " &nonempty, &Ap, &Aj, &Ax, desc)") ;
    GB_BURBLE_START ("GxB_Matrix_export_CSR") ;
    GB_EXPORT_CHECK ;

    GB_RETURN_IF_NULL (Ap) ;
    GB_RETURN_IF_NULL (Aj) ;
    GB_RETURN_IF_NULL (Ax) ;

    //--------------------------------------------------------------------------
    // export the matrix
    //--------------------------------------------------------------------------

    // ensure the matrix is in standard CSR format
    (*A)->hyper_ratio = GB_NEVER_HYPER ;
    if ((*A)->is_csc)
    { 
        // A = A', done in place, to put A in CSR format
        GBBURBLE ("(transpose) ") ;
        GB_OK (GB_transpose (NULL, NULL, false, (*A), NULL, Context)) ;
    }
    if ((*A)->is_hyper)
    { 
        // convert A from hypersparse to standard format
        GB_OK (GB_to_nonhyper ((*A), Context)) ;
    }

    ASSERT_MATRIX_OK ((*A), "A export: standard CSR", GB0) ;
    ASSERT (!((*A)->is_csc)) ;
    ASSERT (!((*A)->is_hyper)) ;

    if ((*A)->nvec_nonempty < 0)
    { 
        // count # of non-empty vectors
        (*A)->nvec_nonempty = GB_nvec_nonempty (*A, Context) ;
    }
    (*nonempty) = (*A)->nvec_nonempty ;

    // export the content and remove it from A
    (*Ap) = (GrB_Index *) (*A)->p ;
    (*A)->p = NULL ;
    if ((*nvals) > 0)
    { 
        (*Aj) = (GrB_Index *) (*A)->i ;
        (*Ax) = (*A)->x ;
        (*A)->i = NULL ;
        (*A)->x = NULL ;
    }
    else
    { 
        (*Aj) = NULL ;
        (*Ax) = NULL ;
    }
    ASSERT ((*A)->h == NULL) ;

    //--------------------------------------------------------------------------
    // export is successful
    //--------------------------------------------------------------------------

    // free the matrix header; do not free the exported content of the matrix,
    // which has already been removed above.
    GB_MATRIX_FREE (A) ;
    ASSERT (*A == NULL) ;
    GB_BURBLE_END ;
    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GxB_Matrix_export_HyperCSC: export a matrix in hypersparse CSC format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_export.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_export_HyperCSC  // export and free a hypersparse CSC matrix
(
    GrB_Matrix *A,          // handle of matrix to export and free
    GrB_Type *type,         // type of matrix exported
    GrB_Index *nrows,       // matrix dimension is nrows-by-ncols
    GrB_Index *ncols,
    GrB_Index *nvals,       // number of entries in the matrix
    // hypersparse CSC format:
    int64_t *nonempty,      // number of columns in Ah with at least one entry
    GrB_Index *nvec,        // number of columns in Ah list
    GrB_Index **Ah,         // list of size nvec of columns that appear in A
    GrB_Index **Ap,         // columns "pointers", size nvec+1
    GrB_Index **Ai,         // row indices, size nvals
    void      **Ax,         // values, size nvals
    const GrB_Descriptor desc       // descriptor for # of threads to use
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Matrix_export_HyperCSC (&A, &type, &nrows, &ncols, &nvals,"
        " &nonempty, &nvec, &Ah, &Ap, &Ai, &Ax, desc)") ;
    GB_BURBLE_START ("GxB_Matrix_export_HyperCSC") ;
    GB_EXPORT_CHECK ;

    GB_RETURN_IF_NULL (nvec) ;
    GB_RETURN_IF_NULL (Ah) ;
    GB_RETURN_IF_NULL (Ap) ;
    GB_RETURN_IF_NULL (Ai) ;
    GB_RETURN_IF_NULL (Ax) ;

    //--------------------------------------------------------------------------
    // export the matrix
    //--------------------------------------------------------------------------

    // ensure the matrix is in hypersparse CSC format
    (*A)->hyper_ratio = GB_ALWAYS_HYPER ;
    if (!((*A)->is_hyper))
    { 
        // convert A from standard to hypersparse format
        GB_OK (GB_to_hyper ((*A), Context)) ;
    }
    if (!((*A)->is_csc))
    {
        // A = A', done in place, to put A in CSC format
        GBBURBLE ("(transpose) ") ;
        GB_OK (GB_transpose (NULL, NULL, true, (*A), NULL, Context)) ;
        // the transpose might make it non-hypersparse (if vdim is 1)
        if (!((*A)->is_hyper))
        { 
            // convert A from standard to hypersparse format
            GB_OK (GB_to_hyper ((*A), Context)) ;
        }
    }

    ASSERT_MATRIX_OK ((*A), "A export: hyper CSC", GB0) ;
    ASSERT ((*A)->is_csc) ;
    ASSERT ((*A)->is_hyper) ;

    if ((*A)->nvec_nonempty < 0)
    { 
        // count # of non-empty vectors
        (*A)->nvec_nonempty = GB_nvec_nonempty (*A, Context) ;
    }
    (*nonempty) = (*A)->nvec_nonempty ;

    // export the content and remove it from A
    (*nvec) = (*A)->nvec ;
    (*Ah) = (GrB_Index *) (*A)->h ;
    (*A)->h = NULL ;
    (*Ap) = (GrB_Index *) (*A)->p ;
    (*A)->p = NULL ;
    if ((*nvals) > 0)
    { 
        (*Ai) = (GrB_Index *) (*A)->i ;
        (*Ax) = (*A)->x ;
        (*A)->i = NULL ;
        (*A)->x = NULL ;
    }
    else
    { 
        (*Ai) = NULL ;
        (*Ax) = NULL ;
    }

    //--------------------------------------------------------------------------
    // export is successful
    //--------------------------------------------------------------------------

    // free the matrix header; do not free the exported content of the matrix,
    // which has already been removed above.
    GB_MATRIX_FREE (A) ;
    ASSERT (*A == NULL) ;
    GB_BURBLE_END ;
    return (info) ;
}


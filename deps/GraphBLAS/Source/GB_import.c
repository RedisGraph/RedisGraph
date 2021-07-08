//------------------------------------------------------------------------------
// GB_import: import a matrix in any format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

GrB_Info GB_import      // import/pack a matrix in any format
(
    bool packing,       // pack if true, create and import false

    GrB_Matrix *A,      // handle of matrix to create, or pack
    GrB_Type type,      // type of matrix to create
    GrB_Index vlen,     // vector length
    GrB_Index vdim,     // vector dimension
    bool is_sparse_vector,      // true if A is a sparse GrB_Vector

    // the 5 arrays:
    GrB_Index **Ap,     // pointers, for sparse and hypersparse formats.
    GrB_Index Ap_size,  // size of Ap in bytes

    GrB_Index **Ah,     // vector indices for hypersparse matrices
    GrB_Index Ah_size,  // size of Ah in bytes

    int8_t **Ab,        // bitmap, for bitmap format only.
    GrB_Index Ab_size,  // size of Ab in bytes

    GrB_Index **Ai,     // indices for hyper and sparse formats
    GrB_Index Ai_size,  // size of Ai in bytes

    void **Ax,          // values
    GrB_Index Ax_size,  // size of Ax in bytes

    // additional information for specific formats:
    GrB_Index nvals,    // # of entries for bitmap format, or for a vector
                        // in CSC format.
    bool jumbled,       // if true, sparse/hypersparse may be jumbled.
    GrB_Index nvec,     // size of Ah for hypersparse format.

    // information for all formats:
    int sparsity,       // hypersparse, sparse, bitmap, or full
    bool is_csc,        // if true then matrix is by-column, else by-row
    bool iso,           // if true then A is iso and only one entry is provided
                        // in Ax, regardless of nvals(A).
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL (A) ;

    if (!packing)
    { 
        (*A) = NULL ;
    }

    GB_RETURN_IF_NULL_OR_FAULTY (type) ;
    if (vlen  > GxB_INDEX_MAX || vdim > GxB_INDEX_MAX ||
        nvals > GxB_INDEX_MAX || nvec > GxB_INDEX_MAX ||
        Ap_size > GxB_INDEX_MAX ||
        Ah_size > GxB_INDEX_MAX || Ab_size > GxB_INDEX_MAX ||
        Ai_size > GxB_INDEX_MAX || Ax_size > GxB_INDEX_MAX)
    { 
        return (GrB_INVALID_VALUE) ;
    }

    if (Ax_size > 0)
    { 
        // Ax and (*Ax) are ignored if Ax_size is zero
        GB_RETURN_IF_NULL (Ax) ;
        GB_RETURN_IF_NULL (*Ax) ;
    }

    bool ok = true ;
    int64_t full_size = 0, Ax_size_for_non_iso ;
    if (sparsity == GxB_BITMAP || sparsity == GxB_FULL)
    {
        ok = GB_Index_multiply ((GrB_Index *) &full_size, vlen, vdim) ;
        if (!ok) full_size = INT64_MAX ;
    }

    switch (sparsity)
    {
        case GxB_HYPERSPARSE : 
            // check Ap and get nvals
            if (nvec > vdim) return (GrB_INVALID_VALUE) ;
            if (Ap_size < (nvec+1) * sizeof (int64_t))
            { 
                return (GrB_INVALID_VALUE) ;
            }
            GB_RETURN_IF_NULL (Ap) ;
            GB_RETURN_IF_NULL (*Ap) ;
            nvals = (*Ap) [nvec] ;
            // check Ah
            GB_RETURN_IF_NULL (Ah) ;
            GB_RETURN_IF_NULL (*Ah) ;
            if (Ah_size < nvec * sizeof (int64_t))
            { 
                return (GrB_INVALID_VALUE) ;
            }
            // check Ai
            if (Ai_size > 0)
            { 
                GB_RETURN_IF_NULL (Ai) ;
                GB_RETURN_IF_NULL (*Ai) ;
            }
            if (Ai_size < nvals * sizeof (int64_t))
            { 
                return (GrB_INVALID_VALUE) ;
            }
            Ax_size_for_non_iso = nvals ;
            break ;

        case GxB_SPARSE : 
            // check Ap and get nvals
            if (!is_sparse_vector)
            {
                // GxB_Vector_import_CSC passes in Ap as a NULL, and nvals as
                // the # of entries in the vector.  All other uses of GB_import
                // pass in Ap for the sparse case
                if (Ap_size < (vdim+1) * sizeof (int64_t))
                { 
                    return (GrB_INVALID_VALUE) ;
                }
                GB_RETURN_IF_NULL (Ap) ;
                GB_RETURN_IF_NULL (*Ap) ;
                nvals = (*Ap) [vdim] ;
            }
            // check Ai
            if (Ai_size > 0)
            { 
                GB_RETURN_IF_NULL (Ai) ;
                GB_RETURN_IF_NULL (*Ai) ;
            }
            if (Ai_size < nvals * sizeof (int64_t))
            { 
                return (GrB_INVALID_VALUE) ;
            }
            Ax_size_for_non_iso = nvals ;
            break ;

        case GxB_BITMAP : 
            // check Ab
            if (!ok) return (GrB_INVALID_VALUE) ;
            if (Ab_size > 0)
            { 
                GB_RETURN_IF_NULL (Ab) ;
                GB_RETURN_IF_NULL (*Ab) ;
            }
            if (nvals > full_size) return (GrB_INVALID_VALUE) ;
            if (Ab_size < full_size) return (GrB_INVALID_VALUE) ;
            Ax_size_for_non_iso = full_size ;
            break ;

        case GxB_FULL : 
            Ax_size_for_non_iso = full_size ;
            break ;

        default: ;
    }

    // check the size of Ax
    if (iso)
    {
        // A is iso: Ax must be non-NULL and large enough to hold a single entry
        GBURBLE ("(iso import) ") ;
        if (*Ax == NULL || Ax_size < type->size)
        { 
            return (GrB_INVALID_VALUE) ;
        }
    }
    else
    {
        // A is non-iso: Ax_size must be zero (and Ax must then be NULL),
        // or Ax_size must be at least as large as Ax_size_for_non_iso
        if (!((Ax_size == 0 && *Ax == NULL) ||
              (Ax_size >= Ax_size_for_non_iso && *Ax != NULL)))
        { 
            return (GrB_INVALID_VALUE) ;
        }
    }

    //--------------------------------------------------------------------------
    // allocate/reuse the header of the matrix
    //--------------------------------------------------------------------------

    if (packing)
    { 
        // clear the content and reuse the header
        GB_phbix_free (*A) ;
    }

    // also create A->p if this is a sparse GrB_Vector
    GrB_Info info = GB_new (A, false, // any sparsity, new user header
        type, vlen, vdim, is_sparse_vector ? GB_Ap_calloc : GB_Ap_null,
        is_csc, sparsity, GB_Global_hyper_switch_get ( ), nvec, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // import the matrix
    //--------------------------------------------------------------------------

    // transplant the user's content into the matrix
    (*A)->magic = GB_MAGIC ;
    (*A)->iso = iso ;   // OK

    switch (sparsity)
    {
        case GxB_HYPERSPARSE : 
            (*A)->nvec = nvec ;

            // import A->h, then fall through to sparse case
            (*A)->h = (int64_t *) (*Ah) ; (*Ah) = NULL ;
            (*A)->h_size = Ah_size ;
            #ifdef GB_DEBUG
            #ifdef GB_MEMDUMP
            printf ("import A->h to memtable: %p\n", (*A)->h) ;
            #endif
            GB_Global_memtable_add ((*A)->h, (*A)->h_size) ;
            #endif

        case GxB_SPARSE : 
            (*A)->jumbled = jumbled ;   // import jumbled status
            (*A)->nvec_nonempty = -1 ;  // not computed; delay until required

            if (is_sparse_vector)
            { 
                // GxB_Vector_import_CSC passes in Ap as NULL
                (*A)->p [1] = nvals ;
            }
            else
            { 
                // import A->p, unless already created for a sparse CSC vector
                (*A)->p = (int64_t *) (*Ap) ; (*Ap) = NULL ;
                (*A)->p_size = Ap_size ;
                #ifdef GB_DEBUG
                #ifdef GB_MEMDUMP
                printf ("import A->p to memtable: %p\n", (*A)->p) ;
                #endif
                GB_Global_memtable_add ((*A)->p, (*A)->p_size) ;
                #endif
            }

            // import A->i
            (*A)->i = (int64_t *) (*Ai) ; (*Ai) = NULL ;
            (*A)->i_size = Ai_size ;
            #ifdef GB_DEBUG
            #ifdef GB_MEMDUMP
            printf ("import A->i to memtable: %p\n", (*A)->i) ;
            #endif
            GB_Global_memtable_add ((*A)->i, (*A)->i_size) ;
            #endif
            break ;

        case GxB_BITMAP : 
            (*A)->nvals = nvals ;

            // import A->b
            (*A)->b = (*Ab) ; (*Ab) = NULL ;
            (*A)->b_size = Ab_size ;
            #ifdef GB_DEBUG
            #ifdef GB_MEMDUMP
            printf ("import A->b to memtable: %p\n", (*A)->b) ;
            #endif
            GB_Global_memtable_add ((*A)->b, (*A)->b_size) ;
            #endif
            break ;

        case GxB_FULL : 
            break ;

        default: ;
    }

    if (Ax != NULL)
    { 
        // import A->x
        (*A)->x = (*Ax) ; (*Ax) = NULL ;
        (*A)->x_size = Ax_size ;
        #ifdef GB_DEBUG
        #ifdef GB_MEMDUMP
        printf ("import A->x to memtable: %p\n", (*A)->x) ;
        #endif
        GB_Global_memtable_add ((*A)->x, (*A)->x_size) ;
        #endif
    }

    //--------------------------------------------------------------------------
    // import is successful
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (*A, "A imported", GB0) ;
    return (GrB_SUCCESS) ;
}


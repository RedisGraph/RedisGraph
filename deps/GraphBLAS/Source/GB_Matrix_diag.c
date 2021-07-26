//------------------------------------------------------------------------------
// GB_Matrix_diag: construct a diagonal matrix from a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_FREE_WORK        \
    GB_phbix_free (T) ;

#define GB_FREE_ALL         \
    GB_FREE_WORK ;          \
    GB_phbix_free (C) ;

#include "GB_diag.h"

GrB_Info GB_Matrix_diag     // construct a diagonal matrix from a vector
(
    GrB_Matrix C,           // output matrix
    const GrB_Matrix V_in,  // input vector (as an n-by-1 matrix)
    int64_t k,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C input for GB_Matrix_diag", GB0) ;
    ASSERT_MATRIX_OK (V_in, "V input for GB_Matrix_diag", GB0) ;
    ASSERT (GB_VECTOR_OK (V_in)) ;       // V_in is a vector on input
    ASSERT (!GB_aliased (C, V_in)) ;     // C and V_in cannot be aliased
    ASSERT (!GB_IS_HYPERSPARSE (V_in)) ; // vectors cannot be hypersparse

    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = GB_clear_static_header (&T_header) ;

    GrB_Type ctype = C->type ;
    GrB_Type vtype = V_in->type ;
    int64_t nrows = GB_NROWS (C) ;
    int64_t ncols = GB_NCOLS (C) ;
    int64_t n = V_in->vlen + GB_IABS (k) ;     // C must be n-by-n

    if (nrows != ncols || nrows != n)
    { 
        GB_ERROR (GrB_DIMENSION_MISMATCH,
            "Input matrix is " GBd "-by-" GBd " but must be "
            GBd "-by-" GBd "\n", nrows, ncols, n, n) ;
    }

    if (!GB_Type_compatible (ctype, vtype))
    { 
        GB_ERROR (GrB_DOMAIN_MISMATCH, "Input vector of type [%s] "
            "cannot be typecast to output of type [%s]\n",
            vtype->name, ctype->name) ;
    }

    //--------------------------------------------------------------------------
    // finish any pending work in V_in and clear the output matrix C
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT (V_in) ;
    GB_phbix_free (C) ;

    //--------------------------------------------------------------------------
    // ensure V is not bitmap
    //--------------------------------------------------------------------------

    GrB_Matrix V ;
    if (GB_IS_BITMAP (V_in))
    { 
        // make a deep copy of V_in and convert to CSC
        // set T->iso = V_in->iso   OK
        GB_OK (GB_dup_worker (&T, V_in->iso, V_in, true, NULL, Context)) ;
        GB_OK (GB_convert_bitmap_to_sparse (T, Context)) ;
        V = T ;
    }
    else
    { 
        // use V_in as-is
        V = V_in ;
    }

    //--------------------------------------------------------------------------
    // allocate C as sparse or hypersparse with vnz entries and vnz vectors
    //--------------------------------------------------------------------------

    // C is sparse if V is dense and k == 0, and hypersparse otherwise
    const int64_t vnz = GB_nnz (V) ;
    const bool V_is_full = GB_is_dense (V) ;
    const int C_sparsity = (V_is_full && k == 0) ? GxB_SPARSE : GxB_HYPERSPARSE;
    const bool C_iso = V->iso ;
    if (C_iso)
    { 
        GBURBLE ("(iso diag) ") ;
    }
    const bool csc = C->is_csc ;
    const float bitmap_switch = C->bitmap_switch ;
    const int sparsity_control = C->sparsity_control ;

    // set C->iso = C_iso   OK
    GB_OK (GB_new_bix (&C, C->static_header,   // prior static or dynamic header
        ctype, n, n, GB_Ap_malloc, csc, C_sparsity, false,
        C->hyper_switch, vnz, vnz, true, C_iso, Context)) ;
    C->sparsity_control = sparsity_control ;
    C->bitmap_switch = bitmap_switch ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format of C and determine position of diagonal
    //--------------------------------------------------------------------------

    if (!csc)
    { 
        // The kth diagonal of a CSC matrix is the same as the (-k)th diagonal
        // of the CSR format, so if C is CSR, negate the value of k.  Then
        // treat C as if it were CSC in the rest of this method.
        k = -k ;
    }

    int64_t kpositive, knegative ;
    if (k >= 0)
    { 
        kpositive = k ;
        knegative = 0 ;
    }
    else
    { 
        kpositive = 0 ;
        knegative = -k ;
    }

    //--------------------------------------------------------------------------
    // get the contents of C and determine # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (vnz, chunk, nthreads_max) ;
    int64_t *restrict Cp = C->p ;
    int64_t *restrict Ch = C->h ;
    int64_t *restrict Ci = C->i ;
    GB_Type_code vcode = vtype->code ;
    GB_Type_code ccode = ctype->code ;
    size_t vsize = vtype->size ;

    //--------------------------------------------------------------------------
    // copy the contents of V into the kth diagonal of C
    //--------------------------------------------------------------------------

    if (C_sparsity == GxB_SPARSE)
    {

        //----------------------------------------------------------------------
        // V is full, or can be treated as full, and k == 0
        //----------------------------------------------------------------------

        // C->x = (ctype) V->x
        GB_cast_matrix (C, V, Context) ;

        // construct Cp and Ci
        int64_t p ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < vnz ; p++)
        { 
            Cp [p] = p ;
            Ci [p] = p ;
        }

    }
    else if (V_is_full)
    {

        //----------------------------------------------------------------------
        // V is full, or can be treated as full, and k != 0
        //----------------------------------------------------------------------

        // C->x = (ctype) V->x
        GB_cast_matrix (C, V, Context) ;

        // construct Cp, Ch, and Ci
        int64_t p ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < vnz ; p++)
        { 
            Cp [p] = p ;
            Ch [p] = p + kpositive ;
            Ci [p] = p + knegative ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // V is sparse
        //----------------------------------------------------------------------

        // C->x = (ctype) V->x
        GB_cast_matrix (C, V, Context) ;

        int64_t *restrict Vi = V->i ;

        // construct Cp, Ch, and Ci
        int64_t p ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < vnz ; p++)
        { 
            Cp [p] = p ;
            Ch [p] = Vi [p] + kpositive ;
            Ci [p] = Vi [p] + knegative ;
        }
    }

    //--------------------------------------------------------------------------
    // finalize the matrix C
    //--------------------------------------------------------------------------

    Cp [vnz] = vnz ;
    C->nvec = vnz ;
    C->nvec_nonempty = vnz ;
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // free workspace, conform C to its desired format, and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C before conform for GB_Matrix_diag", GB0) ;
    GB_OK (GB_conform (C, Context)) ;
    ASSERT_MATRIX_OK (C, "C output for GB_Matrix_diag", GB0) ;
    return (GrB_SUCCESS) ;
}


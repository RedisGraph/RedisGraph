//------------------------------------------------------------------------------
// GB_dup_worker: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = A, making a deep copy.  The header for C may already exist.

// if numeric is false, C->x is allocated but not initialized.

// If *Chandle is not NULL, the header is reused.  It may be a static or
// dynamic header, depending on C->static_header.

#include "GB.h"
#define GB_FREE_ALL ;

GrB_Info GB_dup_worker      // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // output matrix, NULL or existing static/dynamic
    const bool C_iso,       // if true, construct C as iso
    const GrB_Matrix A,     // input matrix to copy
    const bool numeric,     // if true, duplicate the numeric values; if A is
                            // iso, only the first entry is copied, regardless
                            // of C_iso on input
    const GrB_Type ctype,   // type of C, if numeric is false
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (A, "A to duplicate", GB0) ;
    ASSERT (Chandle != NULL) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anz = GB_nnz_held (A) ;
    int64_t *Ap = A->p ;
    int64_t *Ah = A->h ;
    int64_t *Ai = A->i ;
    int8_t  *Ab = A->b ;
    GB_void *Ax = (GB_void *) A->x ;
    int64_t anvec = A->nvec ;
    int64_t anvals = A->nvals ;
    int64_t anvec_nonempty = A->nvec_nonempty ;
    int64_t A_nzombies = A->nzombies ;
    bool A_jumbled = A->jumbled ;
    int sparsity_control = A->sparsity_control ;
    GrB_Type atype = A->type ;

    //--------------------------------------------------------------------------
    // create C
    //--------------------------------------------------------------------------

    // create C; allocate C->p and do not initialize it.
    // C has the exact same sparsity structure as A.

    // allocate a new user header for C if (*Chandle) is NULL, or reuse the
    // existing static or dynamic header if (*Chandle) is not NULL.
    GrB_Matrix C = (*Chandle) ;
    bool C_static_header = (C == NULL) ? false : C->static_header ;
    // set C->iso = C_iso   OK: burble in the caller
    GB_OK (GB_new_bix (Chandle, C_static_header, // new/old/static header
        numeric ? atype : ctype, A->vlen, A->vdim, GB_Ap_malloc, A->is_csc,
        GB_sparsity (A), false, A->hyper_switch, A->plen, anz, true, C_iso,
        Context)) ;
    C = (*Chandle) ;

    //--------------------------------------------------------------------------
    // copy the contents of A into C
    //--------------------------------------------------------------------------

    C->nvec = anvec ;
    C->nvec_nonempty = anvec_nonempty ;
    C->nvals = anvals ;             // for bitmap only
    C->jumbled = A_jumbled ;        // C is jumbled if A is jumbled
    C->nzombies = A_nzombies ;      // zombies can be duplicated
    C->sparsity_control = sparsity_control ;

    if (Ap != NULL)
    { 
        GB_memcpy (C->p, Ap, (anvec+1) * sizeof (int64_t), nthreads_max) ;
    }
    if (Ah != NULL)
    { 
        GB_memcpy (C->h, Ah, anvec * sizeof (int64_t), nthreads_max) ;
    }
    if (Ab != NULL)
    { 
        GB_memcpy (C->b, Ab, anz * sizeof (int8_t), nthreads_max) ;
    }
    if (Ai != NULL)
    {
        GB_memcpy (C->i, Ai, anz * sizeof (int64_t), nthreads_max) ;
    }
    if (numeric)
    { 
        ASSERT (C_iso == A->iso) ;
        ASSERT (C->type == A->type) ;
        GB_memcpy (C->x, Ax, (A->iso ? 1:anz) * atype->size, nthreads_max) ;
    }

    C->magic = GB_MAGIC ;      // C->p and C->h are now initialized
    #ifdef GB_DEBUG
    if (numeric) ASSERT_MATRIX_OK (C, "C duplicate of A", GB0) ;
    #endif

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}


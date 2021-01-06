//------------------------------------------------------------------------------
// GB_dup2: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = A, making a deep copy.  The header for C may already exist.

// if numeric is false, C->x is allocated but not initialized.

#include "GB.h"

GrB_Info GB_dup2            // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // handle of output matrix to create 
    const GrB_Matrix A,     // input matrix to copy
    const bool numeric,     // if true, duplicate the numeric values
    const GrB_Type ctype,   // type of C, if numeric is false
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ_HELD (A) ;
    int64_t *Ap = A->p ;
    int64_t *Ah = A->h ;
    int64_t *Ai = A->i ;
    int8_t  *Ab = A->b ;
    GB_void *Ax = A->x ;
    int64_t anvec = A->nvec ;
    int64_t anvals = A->nvals ;
    int64_t anvec_nonempty = A->nvec_nonempty ;
    bool A_jumbled = A->jumbled ;
    int sparsity = A->sparsity ;
    GrB_Type atype = A->type ;

    //--------------------------------------------------------------------------
    // create C
    //--------------------------------------------------------------------------

    // create C; allocate C->p and do not initialize it.
    // C has the exact same sparsity structure as A.

    // allocate a new header for C if (*Chandle) is NULL, or reuse the
    // existing header if (*Chandle) is not NULL.
    GrB_Matrix C = (*Chandle) ;
    GrB_Info info = GB_new_bix (&C, // same sparsity as A; old or new header
        numeric ? atype : ctype, A->vlen, A->vdim, GB_Ap_malloc, A->is_csc,
        GB_sparsity (A), false, A->hyper_switch, A->plen, anz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // copy the contents of A into C
    //--------------------------------------------------------------------------

    C->nvec = anvec ;
    C->nvec_nonempty = anvec_nonempty ;
    C->nvals = anvals ;             // for bitmap only
    C->jumbled = A_jumbled ;        // C is jumbled if A is jumbled
    C->sparsity = sparsity ;        // copy in the sparsity control

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
        GB_memcpy (C->x, Ax, anz * atype->size, nthreads_max) ;
    }

    C->magic = GB_MAGIC ;      // C->p and C->h are now initialized
    #ifdef GB_DEBUG
    if (numeric) ASSERT_MATRIX_OK (C, "C duplicate of A", GB0) ;
    #endif

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}


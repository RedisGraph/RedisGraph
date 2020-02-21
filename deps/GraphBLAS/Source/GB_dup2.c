//------------------------------------------------------------------------------
// GB_dup2: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
    // C = A
    //--------------------------------------------------------------------------

    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

    // [ create C; allocate C->p and do not initialize it
    // C has the exact same hypersparsity as A.
    GrB_Info info ;
    int64_t anz = GB_NNZ (A) ;

    // allocate a new header for C if (*Chandle) is NULL, or reuse the
    // existing header if (*Chandle) is not NULL.
    GrB_Matrix C = (*Chandle) ;

    GB_CREATE (&C, numeric ? A->type : ctype, A->vlen, A->vdim, GB_Ap_malloc,
        A->is_csc, GB_SAME_HYPER_AS (A->is_hyper), A->hyper_ratio, A->plen,
        anz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    // copy the contents of A into C
    int64_t anvec = A->nvec ;
    C->nvec = anvec ;
    C->nvec_nonempty = A->nvec_nonempty ;
    int64_t *GB_RESTRICT Cp = C->p ;
    int64_t *GB_RESTRICT Ch = C->h ;
    int64_t *GB_RESTRICT Ci = C->i ;
    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t *GB_RESTRICT Ai = A->i ;

    int nthreads = GB_nthreads (anvec, chunk, nthreads_max) ;
    GB_memcpy (Cp, Ap, (anvec+1) * sizeof (int64_t), nthreads) ;
    if (A->is_hyper)
    { 
        GB_memcpy (Ch, Ah, anvec * sizeof (int64_t), nthreads) ;
    }

    nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
    GB_memcpy (Ci, Ai, anz * sizeof (int64_t), nthreads) ;
    if (numeric)
    { 
        GB_memcpy (C->x, A->x, anz * A->type->size, nthreads) ;
    }

    C->magic = GB_MAGIC ;      // C->p and C->h are now initialized ]
    #ifdef GB_DEBUG
    if (numeric) ASSERT_MATRIX_OK (C, "C duplicate of A", GB0) ;
    #endif

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}


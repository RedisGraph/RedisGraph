//------------------------------------------------------------------------------
// GB_create: create a matrix and allocate space
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Creates a matrix (with GB_new), then allocates a given space for indices and
// values.

// This function is called via the GB_CREATE(...) macro.

// Ahandle must be non-NULL on input.

// If *Ahandle is NULL on input:

//      A new header for the matrix A is allocated.  If successful, *Ahandle
//      points to the new handle, and its contents, on output.  If an
//      out-of-memory condition occurs, the header is freed and *Ahandle is
//      NULL on output.

// If *Ahandle is not NULL on input:

//      The existing header for A is used.  The pointer *Ahandle itself is not
//      modified on output, either on success or failure.  If successful, the
//      content of A has been created.  If an out-of-memory condition occurs,
//      the preexisting header is not freed and *Ahandle is unmodified on
//      output.

// To see where these options are used in SuiteSparse:GraphBLAS:
// grep "allocate a new header"
// which shows all uses of GB_new and GB_create

#include "GB.h"

GrB_Info GB_create              // create a new matrix, including A->i and A->x
(
    GrB_Matrix *Ahandle,        // output matrix to create
    const GrB_Type type,        // type of output matrix
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int hyper_option,     // 1:hyper, 0:nonhyper, -1:auto
    const double hyper_ratio,   // A->hyper_ratio, unless auto
    const int64_t plen,         // size of A->p and A->h, if hypersparse
    const int64_t anz,          // number of nonzeros the matrix must hold
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Ahandle != NULL) ;

    //--------------------------------------------------------------------------
    // allocate the header and the vector pointers
    //--------------------------------------------------------------------------

    bool preexisting_header = (*Ahandle != NULL) ;
    GrB_Info info = GB_new (Ahandle, type, vlen, vdim, Ap_option,
        is_csc, hyper_option, hyper_ratio, plen, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory.  If *Ahandle was non-NULL on input, it has not
        // been freed by GB_new.
        ASSERT (preexisting_header == (*Ahandle != NULL)) ;
        return (info) ;
    }

    GrB_Matrix A = (*Ahandle) ;

    //--------------------------------------------------------------------------
    // allocate the indices and values
    //--------------------------------------------------------------------------

    info = GB_ix_alloc (A, anz, numeric, Context) ;
    if (info != GrB_SUCCESS)
    {
        // out of memory
        // GB_ix_alloc has already freed all content of A
        if (!preexisting_header)
        { 
            // also free the header *Ahandle itself
            GB_MATRIX_FREE (Ahandle) ;
            ASSERT (*Ahandle == NULL) ;
        }
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GB_Matrix_dup: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A, making a deep copy.  Not user-callable; this function does the work
// for user-callable functions GrB_*_dup.

// (*handle) and A might be identical, with GrB_Matrix_dup (&A, A), so the
// final output is written into the handle only at the very last step.  The
// input matrix A will be lost, and will result in a memory leak, unless the
// user application does:

//  B = A ;
//  GrB_Matrix (&A, A) ;
//  GrB_free (&A) ;
//  GrB_free (&B) ;

// A is the new copy and B is the old copy.  Each should be freed when done.

#include "GB.h"

GrB_Info GB_Matrix_dup      // make an exact copy of a matrix
(
    GrB_Matrix *handle,     // handle of output matrix to create
    const GrB_Matrix A      // input matrix to copy
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (handle != NULL) ;
    ASSERT_OK (GB_check (A, "A to duplicate", 0)) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (A) ;

    // It would also be possible to copy the pending tuples instead.  This
    // might be useful if the input matrix has just a few of them, and then
    // further calls to setElement will be done on the output matrix C.  On the
    // other hand, if A has lots of pending tuples, C will inherit them, and it
    // will double the work needed to assemble both sets of identical tuples.

    // Copying zombies is easy; this code does it already with almost no
    // change (would need to just set the # of zombies in C).

    //--------------------------------------------------------------------------
    // C = A
    //--------------------------------------------------------------------------

    // [ [ create C; malloc C->p and do not initialize it
    double memory = GBYTES (A->ncols + 1, sizeof (int64_t)) ;
    GrB_Info info ;
    GrB_Matrix C ;
    GB_NEW (&C, A->type, A->nrows, A->ncols, false, true) ;

    if (info != GrB_SUCCESS)
    {
        (*handle) = NULL ;
        return (info) ;
    }

    // quick return if A is empty
    if (A->nzmax == 0)
    {
        // both the input matrix A and the output matrix C are empty
        GB_Matrix_clear (C) ;
        // C is now intialized ]
        (*handle) = C ;
        return (REPORT_SUCCESS) ;
    }

    // allocate the content of C
    int64_t nnz = NNZ (A) ;
    if (!GB_Matrix_alloc (C, nnz, true, &memory))
    {
        // out of memory
        GB_MATRIX_FREE (&C) ;
        (*handle) = NULL ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    // copy the contents of A into C
    memcpy (C->p, A->p, (A->ncols+1) * sizeof (int64_t)) ;
    C->magic = MAGIC ;      // C now initialized ]
    memcpy (C->i, A->i, nnz * sizeof (int64_t)) ;
    memcpy (C->x, A->x, nnz * A->type->size) ;

    ASSERT_OK (GB_check (C, "C duplicate of A", 0)) ;
    (*handle) = C ;
    return (REPORT_SUCCESS) ;
}


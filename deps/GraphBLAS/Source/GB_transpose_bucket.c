//------------------------------------------------------------------------------
// GB_transpose_bucket: transpose and optionally typecast and/or apply operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A' or op(A').  Optionally typecasts from A->type to the new type
// ctype, and/or optionally applies a unary operator.  No error checking is
// done by this function except for out-of-memory conditions.  Returns true if
// successful, or false if out of memory.  This function is not user-callable;
// use GrB_transpose or GrB_apply instead.

// If an operator z=op(x) is provided, the type of z must be the same as the
// type of C.  The type of A must be compatible with the type of of x (A is
// typecasted into the type of x).  These conditions must be checked in the
// caller.

// The input matrix A may have jumbled row indices; this is OK.
// The output matrix C will always have sorted row indices.

// This function is agnostic for the CSR/CSC format of C and A.  C_is_csc is
// defined by the caller and assigned to C->is_csc, but otherwise unused.
// A->is_csc is ignored.

// The input can be hypersparse or non-hypersparse.  The output is
// always non-hypersparse.

// The result is never shallow.

// If A is m-by-n in CSC format, with k nonzeros, the time and memory taken is
// O(m+n+k) if A is non-hypersparse, or O(m+k) if hypersparse.  This is fine if
// most rows and columns of A are non-empty, but can be very costly if A or A'
// is hypersparse.  In particular, if A is a non-hypersparse column vector with
// m >> k, the time and memory is O(m), which can be huge.  Thus, for
// hypersparse matrices, or for very sparse matrices, the qsort method should
// be used instead (see GB_transpose).

#include "GB.h"

GrB_Info GB_transpose_bucket    // bucket transpose; typecast and apply op
(
    GrB_Matrix *Chandle,        // output matrix (unallocated on input)
    const GrB_Type ctype,       // type of output matrix C
    const bool C_is_csc,        // format of output matrix C
    const GrB_Matrix A,         // input matrix
    const GrB_UnaryOp op,       // operator to apply, NULL if no operator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    (*Chandle) = NULL ;
    ASSERT_OK (GB_check (ctype, "ctype for transpose", GB0)) ;
    // OK if the matrix A is jumbled; this function is intended to sort it.
    ASSERT_OK_OR_JUMBLED (GB_check (A, "A input for transpose_bucket", GB0)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;

    if (op != NULL)
    { 
        ASSERT_OK (GB_check (op, "op for transpose", GB0)) ;
        ASSERT (ctype == op->ztype) ;
        ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    }

    //--------------------------------------------------------------------------
    // allocate C: always non-hypersparse
    //--------------------------------------------------------------------------

    // The bucket transpose only works when C is not hypersparse.
    // A can be hypersparse.

    int64_t anz = GB_NNZ (A) ;

    // [ C->p is malloc'd but not initialized.  It is NON-hypersparse.
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, ctype, A->vdim, A->vlen, GB_Ap_malloc, C_is_csc,
        GB_FORCE_NONHYPER, A->hyper_ratio, A->vlen, anz, true) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    int64_t *rowcount = NULL ;
    GB_CALLOC_MEMORY (rowcount, A->vlen + 1, sizeof (int64_t)) ;
    if (rowcount == NULL)
    { 
        // out of memory
        double memory = GBYTES (A->vlen + 1, sizeof (int64_t)) ;
        GB_MATRIX_FREE (&C) ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // symbolic analysis
    //--------------------------------------------------------------------------

    // compute the row counts of A
    const int64_t *Ai = A->i ;
    for (int64_t p = 0 ; p < anz ; p++)
    { 
        rowcount [Ai [p]]++ ;
    }

    // compute the vector pointers for C
    C->nvec_nonempty = GB_cumsum (C->p, rowcount, A->vlen) ;
    C->magic = GB_MAGIC ;      // C is now initialized ]

    //--------------------------------------------------------------------------
    // transpose A into C
    //--------------------------------------------------------------------------

    // transpose both the pattern and the values
    if (op == NULL)
    { 
        // do not apply an operator; optional typecast to ctype
        GB_transpose_ix (rowcount, C->i, C->x, ctype, A) ;
    }
    else
    { 
        // apply an operator, C has type op->ztype
        GB_transpose_op (rowcount, C->i, C->x, op, A) ;
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_FREE_MEMORY (rowcount, A->vlen + 1, sizeof (int64_t)) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C transpose of A", GB0)) ;
    ASSERT (!C->is_hyper) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}


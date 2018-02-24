//------------------------------------------------------------------------------
// GB_Matrix_transpose: transpose and optionally typecast and/or apply operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A' or op(A').  Optionally typecasts from A->type to the new type
// C->type, and/or optionally applies a unary operator.  No error checking is
// done by this function except for out-of-memory conditions.  Returns true if
// successful, or false if out of memory.  This function is not user-callable;
// use GrB_transpose or GrB_apply instead.

// If an operator z=op(x) is provided, the type of z must be the same as the
// type of C.  The type of A must be compatible with the type of of x (A is
// typecasted into the type of x).  These conditions must be checked in the
// caller.

// If numeric is false, only the pattern is transposed, and C->x is left NULL.

// The input matrix A may have jumbled row indices; this is OK.
// The output matrix C will always have sorted row indices.

#include "GB.h"

GrB_Info GB_Matrix_transpose    // transpose, optionally typecast and apply op
(
    GrB_Matrix C,               // output matrix
    const GrB_Matrix A,         // input matrix
    const GrB_UnaryOp op,       // operator to apply, NULL if no operator
    const bool numeric          // if true, do the numeric values
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // [ C need not be initialized, just the column pointers present
    ASSERT (C != NULL && C->p != NULL && !C->p_shallow) ;
    // OK if the matrix A is jumbled; this function is intended to sort it.
    ASSERT_OK_OR_JUMBLED (GB_check (A, "A input for transpose", 0)) ;
    // C is about to be freed; so it can have pending tuples and zombies;
    // don't bother to assert it however.
    // A cannot have pending tuples or zombies
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (C->nrows == A->ncols && C->ncols == A->nrows) ;

    if (numeric && op != NULL)
    {
        ASSERT_OK (GB_check (op, "op for transpose", 0)) ;
        ASSERT (C->type == op->ztype) ;
        ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    }

    //--------------------------------------------------------------------------
    // allocate C and the workspace
    //--------------------------------------------------------------------------

    int64_t anz = NNZ (A) ;

    if (anz == 0)
    {
        // no work to do
        GB_Matrix_clear (C) ;
        return (REPORT_SUCCESS) ;
    }

    double memory = 0 ;
    if (!GB_Matrix_alloc (C, anz, numeric, &memory))
    {
        // out of memory
        GB_Matrix_clear (C) ;
        GB_Work_free ( ) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    // ensure Work is large enough
    memory += GBYTES (A->nrows+1, sizeof (int64_t)) ;
    if (!GB_Work_alloc (A->nrows + 1, sizeof (int64_t)))
    {
        // out of memory
        GB_Matrix_clear (C) ;
        GB_Work_free ( ) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    //--------------------------------------------------------------------------
    // clear rowcount
    //--------------------------------------------------------------------------

    int64_t *rowcount = (int64_t *) GB_thread_local.Work ;
    for (int64_t i = 0 ; i < A->nrows ; i++)
    {
        rowcount [i] = 0 ;
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

    // compute the column pointers for C
    GB_cumsum (C->p, rowcount, A->nrows) ;
    C->magic = MAGIC ;      // C is now initialized ]

    //--------------------------------------------------------------------------
    // transpose A into C
    //--------------------------------------------------------------------------

    if (numeric)
    {
        // transpose both the pattern and the values
        if (op == NULL)
        {
            // do not apply an operator; optional typecast to C->type
            GB_transpose_ix (A->p, A->i, A->x, A->type,
                           rowcount, C->i, C->x, A->ncols, C->type) ;
        }
        else
        {
            // apply an operator, C has type op->ztype
            GB_transpose_op (A->p, A->i, A->x, A->type,
                           rowcount, C->i, C->x, A->ncols, op) ;
        }
    }
    else
    {
        // transpose just the pattern
        GB_transpose_pattern (A->p, A->i, rowcount, C->i, A->ncols) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C transpose of A", 0)) ;

    return (REPORT_SUCCESS) ;
}


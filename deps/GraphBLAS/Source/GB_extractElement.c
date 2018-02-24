//------------------------------------------------------------------------------
// GB_extractElement: x = A(i,j)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract the value of single scalar, x = A(i,j), typecasting from the type of
// A to the type of x, as needed.  Not user-callable; does the work for all
// GrB_*_extractElement* functions.

// Returns GrB_SUCCESS if A(i,j) is present, and sets x to its value.
// Returns GrB_NO_VALUE if A(i,j) is not present, and x is unmodified.

// The method used is a binary search of the column A(:,j), which is very fast.
// Logging the GrB_NO_VALUE status with the ERROR (...) macro is likely much
// slower than searching for the entry.  Thus, a specialized macro,
// REPORT_NO_VALUE, is used, which simply logs the status and the row and
// column indices.

#include "GB.h"

GrB_Info GB_extractElement      // extract a single entry, x = A(i,j)
(
    void *x,                    // scalar to extract, not modified if not found
    const GB_Type_code xcode,   // type of the scalar x
    const GrB_Matrix A,         // matrix to extract a scalar from
    const GrB_Index i,          // row index
    const GrB_Index j           // column index
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // delete any lingering zombies and assemble any pending tuples
    // do this as early as possible (see Table 2.4 in spec)
    ASSERT (A != NULL) ;
    APPLY_PENDING_UPDATES (A) ;
    RETURN_IF_NULL (x) ;
    ASSERT (xcode <= GB_UDT_code) ;

    // check row and column indices
    if (i >= A->nrows)
    {
        return (ERROR (GrB_INVALID_INDEX, (LOG,
            "Row index "GBu" out of range; must be < "GBd, i, A->nrows))) ;
    }
    if (j >= A->ncols)
    {
        return (ERROR (GrB_INVALID_INDEX, (LOG,
            "Column index "GBu" out of range; must be < "GBd, j, A->ncols))) ;
    }

    // xcode and A must be compatible
    if (!GB_Type_code_compatible (xcode, A->type->code))
    {
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "entry A(i,j) of type [%s] cannot be typecast\n"
            "to output scalar x of type [%s]",
            A->type->name, GB_code_string (xcode)))) ;
    }

    if (NNZ (A) == 0)
    {
        // quick return
        return (REPORT_NO_VALUE (i, j)) ;
    }

    //--------------------------------------------------------------------------
    // binary search in A(:,j) for row index i
    //--------------------------------------------------------------------------

    const int64_t *Ap = A->p ;
    int64_t pleft = Ap [j] ;
    int64_t pright = Ap [j+1] - 1 ;

    if (pleft > pright)
    {
        // no entries in A (:,j)
        return (REPORT_NO_VALUE (i, j)) ;
    }

    // A->i is int64_t but it is typecasted to unsigned int64 since i is
    // uint64.  This is safe since all indices in A->i are <= GB_INDEX_MAX,
    // and it might speed up the comparison in the binary search to make the
    // types equal.
    const GrB_Index *Ai = (GrB_Index *) A->i ;

    // Time taken for this step is at most O(log(nnz(A(:,j))).

    bool found ;
    GB_BINARY_SEARCH (i, Ai, pleft, pright, found) ;

    if (found)
    {
        size_t asize = A->type->size ;
        // found A (i,j), return its value
        if (xcode == GB_UDT_code || xcode == A->type->code)
        {
            // copy the values without typecasting
            memcpy (x, A->x +(pleft*asize), asize) ;
        }
        else
        {
            // typecast the value from A into x
            GB_cast_array (x, xcode, A->x +(pleft*asize), A->type->code, 1) ;
        }
        return (REPORT_SUCCESS) ;
    }
    else
    {
        // A (i,j) not found.  This is not an error, but an indication to the
        // user than A (i,j) is not present in the matrix.  The matrix does not
        // keep track of its identity value; that depends on the semiring.  So
        // the user would need to interpret this status of 'no value' and take
        // whatever action is appropriate.
        return (REPORT_NO_VALUE (i, j)) ;
    }
}


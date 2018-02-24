//------------------------------------------------------------------------------
// GB_extractTuples: extract all the tuples from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extracts all tuples from a matrix, like [I,J,X] = find (A).  If any
// parameter I, J and/or X is NULL, then that component is not extracted.  The
// size of the I, J, and X arrays (those that are not NULL) is given by nvals,
// which must be at least as large as GrB_nvals (&nvals, A).  The values in the
// matrix are typecasted to the type of X, as needed.

// If all arrays I, J, X are NULL, this function does nothing except to force
// all pending tuples to be assembled.  This is an intended side effect.

// This function is not user-callable.  It does the work for the user-callable
// GrB_*_extractTuples functions.

#include "GB.h"

GrB_Info GB_extractTuples       // extract all tuples from a matrix
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    void *X,                    // array for returning values of tuples
    GrB_Index *p_nvals,         // I,J,X size on input; # tuples on output
    const GB_Type_code xcode,   // type of array X
    const GrB_Matrix A          // matrix to extract tuples from
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // delete any lingering zombies and assemble any pending tuples
    // do this as early as possible (see Table 2.4 in spec)
    ASSERT (A != NULL) ;
    ASSERT (p_nvals != NULL) ;
    APPLY_PENDING_UPDATES (A) ;
    ASSERT (xcode <= GB_UDT_code) ;

    // xcode and A must be compatible
    if (!GB_Type_code_compatible (xcode, A->type->code))
    {
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "entries in A of type [%s] cannot be typecast\n"
            "to output array X of type [%s]",
            A->type->name, GB_code_string (xcode)))) ;
    }

    ASSERT_OK (GB_check (A, "A to extract", 0)) ;

    int64_t anz = NNZ (A) ;

    if (anz == 0)
    {
        // no work to do
        return (REPORT_SUCCESS) ;
    }

    int64_t nvals = *p_nvals ;          // size of I,J,X on input

    if (nvals < anz && (I != NULL || J != NULL | X != NULL))
    {
        // output arrays are not big enough
        return (ERROR (GrB_INSUFFICIENT_SPACE, (LOG,
            "output arrays I,J,X are not big enough: nvals "GBu" < "
            "number of entries "GBd, nvals, anz))) ;
    }

    //--------------------------------------------------------------------------
    // extract the row indices
    //--------------------------------------------------------------------------

    if (I != NULL)
    {
        memcpy (I, A->i, anz * sizeof (int64_t)) ;
    }

    //--------------------------------------------------------------------------
    // extract the column indices
    //--------------------------------------------------------------------------

    if (J != NULL)
    {
        const int64_t *Ap = A->p ;
        for (int64_t j = 0 ; j < A->ncols ; j++)
        {
            for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
            {
                J [p] = j ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // extract the values, typecasting as needed
    //--------------------------------------------------------------------------

    if (X != NULL)
    {
        if (xcode == GB_UDT_code || xcode == A->type->code)
        {
            // Copy the values without typecasting.  For user-defined types,
            // the (void *) X array is assumed to point to values of the right
            // user-defined type, but this can't be checked.  For built-in
            // types, xcode has already been determined by the type of X in the
            // function signature of the caller.
            memcpy (X, A->x, anz * A->type->size) ;
        }
        else
        {
            // typecast the values from A into X, for built-in types only
            GB_cast_array (X, xcode, A->x, A->type->code, anz) ;
        }
    }

    //--------------------------------------------------------------------------
    // return the number of tuples extracted
    //--------------------------------------------------------------------------

    *p_nvals = anz ;            // number of tuples extracted

    return (REPORT_SUCCESS) ;
}


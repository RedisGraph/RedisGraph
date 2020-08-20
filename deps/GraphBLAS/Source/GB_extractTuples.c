//------------------------------------------------------------------------------
// GB_extractTuples: extract all the tuples from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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
    GrB_Index *I_out,           // array for returning row indices of tuples
    GrB_Index *J_out,           // array for returning col indices of tuples
    void *X,                    // array for returning values of tuples
    GrB_Index *p_nvals,         // I,J,X size on input; # tuples on output
    const GB_Type_code xcode,   // type of array X
    const GrB_Matrix A,         // matrix to extract tuples from
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // delete any lingering zombies and assemble any pending tuples
    // do this as early as possible (see Table 2.4 in spec)
    ASSERT (A != NULL) ;
    ASSERT (p_nvals != NULL) ;
    GB_WAIT (A) ;
    ASSERT (xcode <= GB_UDT_code) ;

    // xcode and A must be compatible
    if (!GB_code_compatible (xcode, A->type->code))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "entries in A of type [%s] cannot be typecast\n"
            "to output array X of type [%s]",
            A->type->name, GB_code_string (xcode)))) ;
    }

    ASSERT_MATRIX_OK (A, "A to extract", GB0) ;

    int64_t anz = GB_NNZ (A) ;

    if (anz == 0)
    { 
        // no work to do
        (*p_nvals) = 0 ;
        return (GrB_SUCCESS) ;
    }

    int64_t nvals = *p_nvals ;          // size of I,J,X on input

    if (nvals < anz && (I_out != NULL || J_out != NULL || X != NULL))
    { 
        // output arrays are not big enough
        return (GB_ERROR (GrB_INSUFFICIENT_SPACE, (GB_LOG,
            "output arrays I,J,X are not big enough: nvals "GBd" < "
            "number of entries "GBd, nvals, anz))) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + A->nvec, chunk, nthreads_max) ;

    //-------------------------------------------------------------------------
    // handle the CSR/CSC format
    //--------------------------------------------------------------------------

    GrB_Index *I, *J ;
    if (A->is_csc)
    { 
        I = I_out ;
        J = J_out ;
    }
    else
    { 
        I = J_out ;
        J = I_out ;
    }

    //--------------------------------------------------------------------------
    // extract the row indices
    //--------------------------------------------------------------------------

    if (I != NULL)
    { 
        GB_memcpy (I, A->i, anz * sizeof (int64_t), nthreads) ;
    }

    //--------------------------------------------------------------------------
    // extract the column indices
    //--------------------------------------------------------------------------

    if (J != NULL)
    {
        if (!GB_extract_vector_list ((int64_t *) J, A, nthreads))
        { 
            // out of memory
            return (GB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // extract the values, typecasting as needed
    //--------------------------------------------------------------------------

    if (X != NULL)
    {
        if (xcode > GB_FP64_code || xcode == A->type->code)
        { 
            // Copy the values without typecasting.  For user-defined types,
            // the (void *) X array is assumed to point to values of the right
            // user-defined type, but this can't be checked.  For built-in
            // types, xcode has already been determined by the type of X in the
            // function signature of the caller.
            GB_memcpy (X, A->x, anz * A->type->size, nthreads) ;
        }
        else
        { 
            // typecast the values from A into X, for built-in types only
            GB_cast_array (X, xcode, A->x, A->type->code, anz, Context) ;
        }
    }

    //--------------------------------------------------------------------------
    // return the number of tuples extracted
    //--------------------------------------------------------------------------

    *p_nvals = anz ;            // number of tuples extracted

    return (GrB_SUCCESS) ;
}


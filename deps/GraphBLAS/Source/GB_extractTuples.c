//------------------------------------------------------------------------------
// GB_extractTuples: extract all the tuples from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extracts all tuples from a matrix, like [I,J,X] = find (A).  If any
// parameter I, J and/or X is NULL, then that component is not extracted.  The
// size of the I, J, and X arrays (those that are not NULL) is given by nvals,
// which must be at least as large as GrB_nvals (&nvals, A).  The values in the
// matrix are typecasted to the type of X, as needed.

// This function does the work for the user-callable GrB_*_extractTuples
// functions.

#include "GB.h"

#define GB_FREE_ALL         \
{                           \
    GB_FREE (Ap) ;          \
    GB_FREE (X_bitmap) ;    \
}

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

    GrB_Info info ;
    GB_void *GB_RESTRICT X_bitmap = NULL ; 
    int64_t *GB_RESTRICT Ap = NULL ; 

    ASSERT_MATRIX_OK (A, "A to extract", GB0) ;
    ASSERT (p_nvals != NULL) ;

    // delete any lingering zombies and assemble any pending tuples;
    // allow A to remain jumbled
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (A) ;

    GB_BURBLE_DENSE (A, "(A %s) ") ;
    ASSERT (xcode <= GB_UDT_code) ;
    const GB_Type_code acode = A->type->code ;

    // xcode and A must be compatible
    if (!GB_code_compatible (xcode, acode))
    { 
        return (GrB_DOMAIN_MISMATCH) ;
    }

    const int64_t anz = GB_NNZ (A) ;
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
        return (GrB_INSUFFICIENT_SPACE) ;
    }

    const size_t asize = A->type->size ;

    //-------------------------------------------------------------------------
    // determine the number of threads to use
    //-------------------------------------------------------------------------

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
    // bitmap case
    //--------------------------------------------------------------------------

    if (GB_IS_BITMAP (A))
    {

        //----------------------------------------------------------------------
        // allocate workspace
        //----------------------------------------------------------------------

        bool need_typecast = (X != NULL) && (xcode != acode) ;
        if (need_typecast)
        { 
            // X must be typecasted
            int64_t anzmax = GB_IMAX (anz, 1) ;
            X_bitmap = GB_MALLOC (anzmax * asize, GB_void) ;
        }
        Ap = GB_MALLOC (A->vdim+1, int64_t) ;
        if (Ap == NULL || (need_typecast && X_bitmap == NULL))
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // extract the tuples
        //----------------------------------------------------------------------

        // TODO: pass xcode to GB_convert_bitmap_worker and let it do the
        // typecasting.  This works for now, however.

        GB_OK (GB_convert_bitmap_worker (Ap, I, J,
            need_typecast ? X_bitmap : X, NULL, A, Context)) ;

        //----------------------------------------------------------------------
        // typecast the result if needed
        //----------------------------------------------------------------------

        if (need_typecast)
        { 
            // typecast the values from X_bitmap into X
            GB_cast_array ((GB_void *) X, xcode, X_bitmap,
                acode, NULL, asize, anz, nthreads) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // sparse, hypersparse, or full case
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // extract the row indices
        //----------------------------------------------------------------------

        if (I != NULL)
        {
            if (A->i == NULL)
            {
                // A is full; construct the row indices
                int64_t avlen = A->vlen ;
                int64_t p ;
                #pragma omp parallel for num_threads(nthreads) schedule(static)
                for (p = 0 ; p < anz ; p++)
                { 
                    I [p] = (p % avlen) ;
                }
            }
            else
            { 
                GB_memcpy (I, A->i, anz * sizeof (int64_t), nthreads) ;
            }
        }

        //----------------------------------------------------------------------
        // extract the column indices
        //----------------------------------------------------------------------

        if (J != NULL)
        {
            if (!GB_extract_vector_list ((int64_t *) J, A, nthreads))
            { 
                // out of memory
                return (GrB_OUT_OF_MEMORY) ;
            }
        }

        //----------------------------------------------------------------------
        // extract the values
        //----------------------------------------------------------------------

        if (X != NULL)
        { 
            // typecast or copy the values from A into X
            GB_cast_array ((GB_void *) X, xcode, (GB_void *) A->x,
                acode, NULL, asize, anz, nthreads) ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result 
    //--------------------------------------------------------------------------

    *p_nvals = anz ;            // number of tuples extracted
    GB_FREE_ALL ;
    return (GrB_SUCCESS) ;
}


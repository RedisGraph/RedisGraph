//------------------------------------------------------------------------------
// GB_extractElement: x = A(row,col)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract the value of single scalar, x = A(row,col), typecasting from the
// type of A to the type of x, as needed.  Not user-callable; does the work for
// all GrB_*_extractElement* functions.

// Returns GrB_SUCCESS if A(row,col) is present, and sets x to its value.
// Returns GrB_NO_VALUE if A(row,col) is not present, and x is unmodified.

#include "GB.h"

GrB_Info GB_extractElement      // extract a single entry, x = A(row,col)
(
    void *x,                    // scalar to extract, not modified if not found
    const GB_Type_code xcode,   // type of the scalar x
    const GrB_Matrix A,         // matrix to extract a scalar from
    const GrB_Index row,        // row index
    const GrB_Index col,        // column index
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // delete any lingering zombies and assemble any pending tuples
    // do this as early as possible (see Table 2.4 in spec)
    ASSERT (A != NULL) ;
    GB_WAIT (A) ;
    GB_RETURN_IF_NULL (x) ;
    ASSERT (xcode <= GB_UDT_code) ;

    // check row and column indices
    if (row >= GB_NROWS (A))
    { 
        return (GB_ERROR (GrB_INVALID_INDEX, (GB_LOG,
            "Row index "GBu" out of range; must be < "GBd,
            row, GB_NROWS (A)))) ;
    }
    if (col >= GB_NCOLS (A))
    { 
        return (GB_ERROR (GrB_INVALID_INDEX, (GB_LOG,
            "Column index "GBu" out of range; must be < "GBd,
            col, GB_NCOLS (A)))) ;
    }

    // xcode and A must be compatible
    if (!GB_code_compatible (xcode, A->type->code))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "entry A(i,j) of type [%s] cannot be typecast\n"
            "to output scalar x of type [%s]",
            A->type->name, GB_code_string (xcode)))) ;
    }

    if (GB_NNZ (A) == 0)
    { 
        // quick return
        return (GrB_NO_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format
    //--------------------------------------------------------------------------

    int64_t i, j ;
    if (A->is_csc)
    { 
        // look for index i in vector j
        i = row ;
        j = col ;
    }
    else
    { 
        // look for index j in vector i
        i = col ;
        j = row ;
    }

    //--------------------------------------------------------------------------
    // binary search in A->h for vector j
    //--------------------------------------------------------------------------

    bool found ;
    int64_t k ;
    if (A->is_hyper)
    {
        // look for vector j in hyperlist A->h [0 ... A->nvec-1]
        const int64_t *Ah = A->h ;
        int64_t pleft = 0 ;
        int64_t pright = A->nvec-1 ;
        GB_BINARY_SEARCH (j, Ah, pleft, pright, found) ;
        if (!found)
        { 
            // vector j is empty
            return (GrB_NO_VALUE) ;
        }
        ASSERT (j == Ah [pleft]) ;
        k = pleft ;
    }
    else
    { 
        k = j ;
    }

    //--------------------------------------------------------------------------
    // binary search in kth vector for index i
    //--------------------------------------------------------------------------

    const int64_t *Ap = A->p ;
    int64_t pleft = Ap [k] ;
    int64_t pright = Ap [k+1] - 1 ;

    if (pleft > pright)
    { 
        // no entries in vector j
        return (GrB_NO_VALUE) ;
    }

    // Time taken for this step is at most O(log(nnz(A(:,j))).
    const int64_t *Ai = A->i ;
    GB_BINARY_SEARCH (i, Ai, pleft, pright, found) ;

    //--------------------------------------------------------------------------
    // extract the element
    //--------------------------------------------------------------------------

    if (found)
    {
        GB_void *Ax = A->x ;
        size_t asize = A->type->size ;
        // found A (row,col), return its value
        if (xcode > GB_FP64_code || xcode == A->type->code)
        { 
            // copy the values without typecasting
            memcpy (x, Ax +(pleft*asize), asize) ;
        }
        else
        { 
            // typecast the value from A into x
            GB_cast_array (x, xcode, Ax +(pleft*asize), A->type->code, 1,
                Context) ;
        }
        return (GrB_SUCCESS) ;
    }
    else
    { 
        // Entry not found.  This is not an error, but an indication to the
        // user that the entry is not present in the matrix.  The matrix does
        // not keep track of its identity value; that depends on the semiring.
        // So the user would need to interpret this status of 'no value' and
        // take whatever action is appropriate.
        return (GrB_NO_VALUE) ;
    }
}


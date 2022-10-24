//------------------------------------------------------------------------------
// GB_Matrix_extractElement: x = A(row,col)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extract the value of single scalar, x = A(row,col), typecasting from the
// type of A to the type of x, as needed.

// Returns GrB_SUCCESS if A(row,col) is present, and sets x to its value.
// Returns GrB_NO_VALUE if A(row,col) is not present, and x is unmodified.

// This template constructs GrB_Matrix_extractElement_[TYPE] for each of the
// 13 built-in types, and the _UDT method for all user-defined types.

// FUTURE: tolerate zombies

GrB_Info GB_EXTRACT_ELEMENT     // extract a single entry, x = A(row,col)
(
    GB_XTYPE *x,                // scalar to extract, not modified if not found
    const GrB_Matrix A,         // matrix to extract a scalar from
    GrB_Index row,              // row index
    GrB_Index col               // column index
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL (x) ;

    // TODO: do not wait unless jumbled.  First try to find the element.
    // If found (live or zombie), no need to wait.  If not found and pending
    // tuples exist, wait and then extractElement again.

    // delete any lingering zombies, assemble any pending tuples, and unjumble
    if (GB_ANY_PENDING_WORK (A))
    { 
        GrB_Info info ;
        GB_WHERE1 (GB_WHERE_STRING) ;
        GB_BURBLE_START ("GrB_Matrix_extractElement") ;
        GB_OK (GB_wait (A, "A", Context)) ;
        GB_BURBLE_END ;
    }

    ASSERT (!GB_ANY_PENDING_WORK (A)) ;

    // look for index i in vector j
    int64_t i, j, nrows, ncols ;
    if (A->is_csc)
    { 
        i = row ;
        j = col ;
        nrows = A->vlen ;
        ncols = A->vdim ;
    }
    else
    { 
        i = col ;
        j = row ;
        nrows = A->vdim ;
        ncols = A->vlen ;
    }

    // check row and column indices
    if (row >= nrows || col >= ncols)
    { 
        return (GrB_INVALID_INDEX) ;
    }

    // GB_XCODE and A must be compatible
    GB_Type_code acode = A->type->code ;
    if (!GB_code_compatible (GB_XCODE, acode))
    { 
        return (GrB_DOMAIN_MISMATCH) ;
    }

    if (GB_nnz (A) == 0)
    { 
        // quick return
        return (GrB_NO_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // find the entry A(i,j)
    //--------------------------------------------------------------------------

    int64_t pleft ;
    bool found ;
    const int64_t *restrict Ap = A->p ;

    if (Ap != NULL)
    {
        // A is sparse or hypersparse
        const int64_t *restrict Ai = A->i ;

        // extract from vector j of a GrB_Matrix
        int64_t k ;
        if (A->h != NULL)
        {
            // A is hypersparse: look for j in hyperlist A->h [0 ... A->nvec-1]
            const int64_t *restrict Ah = A->h ;
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
            // A is sparse: j = k is the kth vector
            k = j ;
        }

        pleft = Ap [k] ;
        int64_t pright = Ap [k+1] - 1 ;

        // binary search in kth vector for index i
        // Time taken for this step is at most O(log(nnz(A(:,j))).
        GB_BINARY_SEARCH (i, Ai, pleft, pright, found) ;
    }
    else
    {
        // A is bitmap or full
        pleft = i + j * A->vlen ;
        const int8_t *restrict Ab = A->b ;
        if (Ab != NULL)
        { 
            // A is bitmap
            found = (Ab [pleft] == 1) ;
        }
        else
        { 
            // A is full
            found = true ;
        }
    }

    //--------------------------------------------------------------------------
    // extract the element
    //--------------------------------------------------------------------------

    if (found)
    {
        #if !defined ( GB_UDT_EXTRACT )
        if (GB_XCODE == acode)
        { 
            // copy A [pleft] into x, no typecasting, for built-in types only.
            GB_XTYPE *restrict Ax = ((GB_XTYPE *) (A->x)) ;
            (*x) = Ax [A->iso ? 0:pleft] ;
        }
        else
        #endif
        { 
            // typecast the value from A [pleft] into x
            size_t asize = A->type->size ;
            void *ax = ((GB_void *) A->x) + (A->iso ? 0 : (pleft*asize)) ;
            GB_cast_scalar (x, GB_XCODE, ax, acode, asize) ;
        }
        // TODO: do not flush if extracting to GrB_Scalar
        #pragma omp flush
        return (GrB_SUCCESS) ;
    }
    else
    { 
        // Entry not found.
        return (GrB_NO_VALUE) ;
    }
}

#undef GB_UDT_EXTRACT
#undef GB_EXTRACT_ELEMENT
#undef GB_XTYPE
#undef GB_XCODE


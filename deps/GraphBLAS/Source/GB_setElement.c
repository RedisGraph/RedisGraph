//------------------------------------------------------------------------------
// GB_setElement: C(row,col) = scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Sets the value of single scalar, C(row,col) = scalar, typecasting from the
// type of scalar to the type of C, as needed.  Not user-callable; does the
// work for all GrB_*_setElement* functions.

// If C(row,col) is already present in the matrix, its value is overwritten
// with the scalar.  Otherwise, if the mode determined by GrB_init is
// non-blocking, the tuple (i,j,scalar) is appended to a list of pending tuples
// to C.  When calling GrB_wait, these pending tuples are assembled.  They are
// also assembled if the mode is blocking.

// GrB_setElement is the same as GrB_*assign with an implied SECOND accum
// operator whose ztype, xtype, and ytype are the same as C, with I=i, J=1, a
// 1-by-1 dense matrix A (where nnz (A) == 1), no mask, mask not complemented,
// C_replace effectively false (its value is ignored), and A transpose
// effectively false (since transposing a scalar has no effect).

// Compare this function with GB_extractElement.

#include "GB_Pending.h"

GrB_Info GB_setElement              // set a single entry, C(row,col) = scalar
(
    GrB_Matrix C,                   // matrix to modify
    void *scalar,                   // scalar to set
    const GrB_Index row,            // row index
    const GrB_Index col,            // column index
    const GB_Type_code scalar_code, // type of the scalar
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;
    GB_RETURN_IF_NULL (scalar) ;

    if (row >= GB_NROWS (C))
    { 
        return (GB_ERROR (GrB_INVALID_INDEX, (GB_LOG,
            "Row index "GBu" out of range; must be < "GBd,
            row, GB_NROWS (C)))) ;
    }
    if (col >= GB_NCOLS (C))
    { 
        return (GB_ERROR (GrB_INVALID_INDEX, (GB_LOG,
            "Column index "GBu" out of range; must be < "GBd,
            col, GB_NCOLS (C)))) ;
    }

    ASSERT (scalar_code <= GB_UDT_code) ;

    GrB_Type ctype = C->type ;
    GB_Type_code ccode = ctype->code ;

    // scalar_code and C must be compatible
    if (!GB_code_compatible (scalar_code, ccode))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "input scalar of type [%s]\n"
            "cannot be typecast to entry of type [%s]",
            GB_code_string (scalar_code), ctype->name))) ;
    }

    // pending tuples and zombies are expected
    ASSERT (GB_PENDING_OK (C)) ; ASSERT (GB_ZOMBIES_OK (C)) ;

    #if GB_BURBLE
    bool burble = GB_Global_burble_get ( ) ;
    double t_burble = 0 ;
    // do not burble when waiting on scalars or empty matrices
    burble = burble && ((C->vlen > 1) || (C->vdim > 1)) ;
    #endif

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format
    //--------------------------------------------------------------------------

    int64_t i, j ;
    if (C->is_csc)
    { 
        // set entry with index i in vector j
        i = row ;
        j = col ;
    }
    else
    { 
        // set entry with index j in vector i
        i = col ;
        j = row ;
    }

    //--------------------------------------------------------------------------
    // binary search in C->h for vector j, or constant time lookup if not hyper
    //--------------------------------------------------------------------------

    int64_t pC_start, pC_end, pleft = 0, pright = C->nvec - 1 ;
    bool found = GB_lookup (C->is_hyper, C->h, C->p, &pleft, pright, j,
        &pC_start, &pC_end) ;

    //--------------------------------------------------------------------------
    // binary search in kth vector for index i
    //--------------------------------------------------------------------------

    bool is_zombie ;
    if (found)
    { 
        // vector j has been found; now look for index i
        pleft = pC_start ;
        pright = pC_end - 1 ;

        // Time taken for this step is at most O(log(nnz(C(:,j))).
        const int64_t *Ci = C->i ;
        GB_BINARY_SEARCH_ZOMBIE (i, Ci, pleft, pright, found, C->nzombies,
            is_zombie) ;
    }

    //--------------------------------------------------------------------------
    // set the element
    //--------------------------------------------------------------------------

    if (found)
    {

        //----------------------------------------------------------------------
        // C (i,j) found
        //----------------------------------------------------------------------

        // if not zombie: action: ( =A ): copy A into C
        // else           action: ( undelete ): bring a zombie back to life

        // found C (i,j), assign its value
        size_t csize = ctype->size ;
        GB_void *Cx = C->x ;
        if (scalar_code >= GB_UDT_code || scalar_code == ccode)
        { 
            // copy the values without typecasting
            memcpy (Cx +(pleft*csize), scalar, csize) ;
        }
        else
        { 
            // typecast scalar into C
            GB_cast_array (Cx +(pleft*csize), ccode, scalar, scalar_code, 1,
                Context) ;
        }

        if (is_zombie)
        {
            // bring the zombie back to life
            ASSERT (C->enqueued) ;
            C->i [pleft] = i ;
            C->nzombies-- ;
            if (C->nzombies == 0 && C->Pending == NULL)
            { 
                // remove from queue if no zombies or pending tuples
                // FUTURE:: may thrash; see GrB_wait.
                GB_CRITICAL (GB_queue_remove (C)) ;
            }
        }

        // the check is fine but just costly even when debugging
        // ASSERT_MATRIX_OK (C, "did C for setElement (found)", GB0) ;
        return (GrB_SUCCESS) ;
    }
    else
    {
        //----------------------------------------------------------------------
        // C (i,j) not found: add a pending tuple
        //----------------------------------------------------------------------

        // action: ( insert )

        // No typecasting can be done.  The new pending tuple must either be
        // the first pending tuple, or its type must match the prior pending
        // tuples.  See GB_subassigner for a complete description.

        // stype is the type of this scalar
        GrB_Type stype = GB_code_type (scalar_code, ctype) ;

        bool wait = false ;

        if (C->Pending == NULL)
        { 
            // the new pending tuple is the first one, so it will define
            // C->type-pending = stype.  No need to wait.
            wait = false ;
        }
        else
        {
            if (stype != C->Pending->type)
            { 
                // the scalar type (stype) must match the type of the
                // prior pending tuples.  If the type is different, prior
                // pending tuples must be assembled first.
                wait = true ;
            }
            else if (!GB_op_is_second (C->Pending->op, ctype))
            { 
                // prior op is not SECOND: setElement uses an implicit
                // SECOND_Ctype operator, which must match the operator of the
                // prior pending tuples.  If it doesn't match, prior pending
                // tuples must be assembled first.
                wait = true ;
            }
        }

        if (wait)
        { 
            // Pending tuples exist.  Either the pending operator is not
            // SECOND_ctype (implicit or explicit), or the type of prior
            // pending tuples is not the same as the type of the scalar.  This
            // new tuple requires both conditions to hold.  All prior tuples
            // must be assembled before this new one can be added.

            #if GB_BURBLE
            if (burble)
            {
                GBBURBLE (" [ *_setElement ") ;
                #if defined ( _OPENMP )
                t_burble = GB_OPENMP_GET_WTIME ;
                #endif
            }
            #endif

            // delete any lingering zombies and assemble the pending tuples
            GB_WAIT (C) ;

            #if GB_BURBLE
            if (burble)
            {
                GB_BURBLE_END ;
            }
            #endif

            ASSERT (C->Pending == NULL) ;

            // repeat the search since the C(i,j) entry may have been in
            // the list of pending tuples.  There are no longer any pending
            // tuples, so this recursion will only happen once.  The
            // pending operator will become the implicit SECOND_ctype,
            // and the type of the pending tuples will become ctype.
            return (GB_setElement (C, scalar, row, col, scalar_code, Context)) ;
        }

        // the new tuple is now compatible with prior tuples, if any
        ASSERT (GB_PENDING_OK (C)) ;
        ASSERT (GB_ZOMBIES_OK (C)) ;

        // C (i,j) must be added to the list of pending tuples.
        // If this is the first pending tuple, then the type of pending tuples
        // becomes the type of this scalar, and the pending operator becomes
        // NULL, which is the implicit SECOND_ctype operator.

        if (!GB_Pending_add (&(C->Pending), scalar, stype, NULL, i, j,
            C->vdim > 1))
        { 
            // out of memory
            GB_PHIX_FREE (C) ;
            return (GB_OUT_OF_MEMORY) ;
        }

        // insert C in the queue if it isn't already queued
        ASSERT (GB_PENDING (C)) ;
        if (!(C->enqueued))
        { 
            GB_CRITICAL (GB_queue_insert (C)) ;
        }

        // if this was the first tuple, then the pending operator and
        // pending type have been defined
        ASSERT (GB_op_is_second (C->Pending->op, ctype)) ;
        ASSERT (C->Pending->type == stype) ;
        ASSERT (C->Pending->size == stype->size) ;

        // this assert is fine, just costly even when debugging
        // ASSERT_MATRIX_OK (C, "did C for setElement (not found)", GB0) ;

        #if GB_BURBLE
        // only burble if GB_wait will be called
        burble = (burble && GB_shall_block (C)) ;
        if (burble)
        {
            GBBURBLE (" [ *_setElement ") ;
            #if defined ( _OPENMP )
            t_burble = GB_OPENMP_GET_WTIME ;
            #endif
        }
        #endif

        GrB_Info info = GB_block (C, Context) ;

        #if GB_BURBLE
        if (burble)
        {
            GB_BURBLE_END ;
        }
        #endif

        return (info) ;
    }
}


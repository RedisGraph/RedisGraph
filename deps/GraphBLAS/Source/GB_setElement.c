//------------------------------------------------------------------------------
// GB_setElement: C(i,j) = x
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Sets the value of single scalar, C(i,j) = x, typecasting from the type of
// x to the type of C, as needed.  Not user-callable; does the work for all
// GrB_*_setElement* functions.

// If C(i,j) is already present in the matrix, its value is overwritten with x.
// Otherwise, if the mode determined by GrB_init is non-blocking, the tuple
// (i,j,x) is appended to a list of pending tuples to C.  When calling
// GrB_wait, these pending tuples are assembled.  They are also assembled if
// the mode is blocking.

// GrB_setElement is the same as GrB_*assign with an implied SECOND accum
// operator whose ztype and ytype are the same as C, with I=i, J=1, and a
// 1-by-1 dense matrix A (NNZ (A) == 1), no Mask, Mask not complemented,
// C_replace effectively false (its value is ignored), and A transpose
// effectively false (since transposing a scalar has no effect).

// Compare this function with GB_extractElement.

#include "GB.h"

GrB_Info GB_setElement          // set a single entry, C(i,j) = x
(
    GrB_Matrix C,               // matrix to modify
    const void *x,              // scalar to set
    const GrB_Index i_in,       // row index
    const GrB_Index j_in,       // column index
    const GB_Type_code xcode    // type of the scalar x
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;
    RETURN_IF_NULL (x) ;

    if (i_in >= C->nrows)
    {
        return (ERROR (GrB_INVALID_INDEX, (LOG,
            "Row index "GBu" out of range; must be < "GBd, i_in, C->nrows))) ;
    }
    if (j_in >= C->ncols)
    {
        return (ERROR (GrB_INVALID_INDEX, (LOG,
            "Column index "GBu" out of range; must be < "GBd, j_in, C->ncols)));
    }

    ASSERT (xcode <= GB_UDT_code) ;

    // xcode = A must be compatible
    if (!GB_Type_code_compatible (xcode, C->type->code))
    {
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "input scalar x of type [%s]\n"
            "cannot be typecast to entry of type [%s]",
            GB_code_string (xcode), C->type->name))) ;
    }

    // pending tuples are expected
    ASSERT (PENDING_OK (C)) ;

    // zombies are expected
    ASSERT (ZOMBIES_OK (C)) ;

    int64_t i = (int64_t) i_in ;
    int64_t j = (int64_t) j_in ;

    //--------------------------------------------------------------------------
    // binary search in C(:,j) for row index i
    //--------------------------------------------------------------------------

    const int64_t *Cp = C->p ;
    int64_t pleft = Cp [j] ;
    int64_t pright = Cp [j+1] - 1 ;
    const int64_t *Ci = C->i ;
    bool found, is_zombie ;

    // Time taken for this step is at most O(log(nnz(C(:,j))).

    // ASSERT_OK (GB_check (C, "C for setElement", 3)) ;

    GB_BINARY_ZOMBIE (i, Ci, pleft, pright, found, C->nzombies, is_zombie) ;

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
        size_t csize = C->type->size ;
        if (xcode == GB_UDT_code || xcode == C->type->code)
        {
            // copy the values without typecasting
            memcpy (C->x +(pleft*csize), x, csize) ;
        }
        else
        {
            // typecast the value from x into C
            GB_cast_array (C->x +(pleft*csize), C->type->code, x, xcode, 1) ;
        }

        if (is_zombie)
        {
            // bring the zombie back to life
            C->i [pleft] = i ;
            C->nzombies-- ;
            if (C->nzombies == 0 && C->npending == 0)
            {
                // remove from queue if zombies goes to 0 and npending is zero
                GB_queue_remove (C) ;
            }
        }

        // ASSERT_OK (GB_check (C, "did C for setElement (found)", 0)) ;
        return (REPORT_SUCCESS) ;
    }
    else
    {
        //----------------------------------------------------------------------
        // C (i,j) not found: add a pending tuple
        //----------------------------------------------------------------------

        // action: ( insert )

        // See GB_subassign_kernel for a complete description of this test.
        // The current operator is the implicit SECOND_Ctype operator.
        // check compatibility of prior pending tuples
        if (C->npending == 0)
        {
            // pending operator is the implicit SECOND_Ctype operator
            C->operator_pending = NULL ;
        }
        else if (!GB_op_is_second (C->operator_pending, C->type))
        {
            // pending tuples exist, but the pending operator is not
            // SECOND_Ctype (implicit or explicit).  This new tuple requires
            // the SECOND_Ctype operator (implicit or explicit).  Prior tuples
            // must be assembled.

            // delete any lingering zombies and assemble any pending tuples
            ASSERT (PENDING (C)) ;
            APPLY_PENDING_UPDATES (C) ;
            ASSERT (!PENDING (C)) ;
            ASSERT (!ZOMBIES (C)) ;
            // ASSERT_OK (GB_check (C, "C setElement (not found)", 0)) ;

            // repeat the search since the C(i,j) entry may have been in
            // the list of pending tuples.  There are no longer any pending
            // tuples, so this recursion will only happen once.
            ASSERT (C->npending == 0) ;
            return (GB_setElement (C, x, i_in, j_in, xcode)) ;
        }

        // the new tuple is now compatible with any prior tuples
        ASSERT (PENDING_OK (C)) ;
        ASSERT (ZOMBIES_OK (C)) ;
        ASSERT (GB_op_is_second (C->operator_pending, C->type)) ;

        // C (i,j) must be added to the list of pending tuples
        GrB_Info info = GB_add_pending (C, x, xcode, i, j) ;
        if (info != GrB_SUCCESS)
        {
            // if GB_add_pending fails, all of C has been cleared
            ASSERT (info == GrB_OUT_OF_MEMORY) ;
            return (info) ;
        }

        // ASSERT_OK (GB_check (C, "did C for setElement (not found)", 0)) ;
        return (GB_block (C)) ;
    }
}


//------------------------------------------------------------------------------
// GB_assign: submatrix assignment: C<Mask>(I,J) = accum (C(I,J),A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// submatrix assignment: C<Mask>(I,J) = accum (C(I,J),A)
// compare/contrast this function with GB_subassign.

// All GrB_*_assign operations rely on this function:  GrB_Matrix_assign,
// GrB_Matrix_assign_TYPE, GrB_Vector_assign, GrB_Vector_assign_TYPE,
// GrB_Row_assign, and GrB_Col_assign.

// Only one of the bool parameters: scalar_expansion, col_assign, and
// row_assign can be true.

// If all are false, this function does the work for GrB_Matrix_assign
// and GrB_Vector_assign.

// If scalar_expansion is true, this function performs scalar assignment (the
// GrB_Matrix_assign_TYPE and GrB_Vector_assign_TYPE functions) in which case
// the input matrix A is ignored (it is NULL), and the scalar is used instead.

// If col_assign is true, this function does the work for GrB_Col_assign.

// If row_assign is true, this function does the work for GrB_Row_assign.

#include "GB.h"

#define FREE_ALL                                    \
{                                                   \
    GB_FREE_MEMORY (I2, ni, sizeof (GrB_Index)) ;   \
    GB_FREE_MEMORY (J2, nj, sizeof (GrB_Index)) ;   \
    GB_MATRIX_FREE (&AT) ;                          \
    GB_MATRIX_FREE (&Mask2) ;                       \
}

GrB_Info GB_assign                  // C<Mask>(I,J) = accum (C(I,J),A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,
    const GrB_Index *I_in,          // row indices
    const GrB_Index ni_in,          // number of row indices
    const GrB_Index *J_in,          // column indices
    const GrB_Index nj_in,          // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    const bool col_assign,          // true for GrB_Col_assign
    const bool row_assign           // true for GrB_Row_assign
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C, Mask, and A checked in the caller
    RETURN_IF_UNINITIALIZED (accum) ;
    RETURN_IF_NULL (I_in) ;         // I = GrB_ALL is not NULL
    RETURN_IF_NULL (J_in) ;         // J = GrB_ALL is not NULL

    if (scalar_expansion)
    {
        // GB_assign_scalar: for scalar expansion, the NULL pointer case has
        // been already checked for user-defined types, and can't be NULL for
        // built-in types.
        ASSERT (scalar != NULL) ;
        ASSERT (A == NULL) ;
        ASSERT (!row_assign && !col_assign) ;
    }
    else
    {
        // GrB_*assign, not scalar:  The user's input matrix has been checked.
        // The pointer to the scalar is NULL.
        ASSERT (scalar == NULL) ;
        ASSERT_OK (GB_check (A, "A for GB_assign", 0)) ;
    }

    ASSERT_OK (GB_check (C, "C input for GB_assign", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_assign", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_assign", 0)) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    GrB_Index *I = (GrB_Index *) I_in ;
    GrB_Index *J = (GrB_Index *) J_in ;
    int64_t ni = ni_in ;
    int64_t nj = nj_in ;

    if (I == GrB_ALL)
    {
        // if I is GrB_ALL, this denotes that I is ":"
        ni = C->nrows ;
    }
    if (J == GrB_ALL)
    {
        // if J is GrB_ALL, this denotes that J is ":"
        nj = C->ncols ;
    }

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // check domains and dimensions for C<Mask>(I,J) = accum (C(I,J),A)
    //--------------------------------------------------------------------------

    // GB_compatible is not used since most of it is slightly different here
    if (accum != NULL)
    {
        // C<Mask>(I,J) = accum (C(I,J),A)
        info = GB_BinaryOp_compatible (accum, C->type, C->type,
            (scalar_expansion) ? NULL : A->type,
            (scalar_expansion) ? scalar_code : 0) ;
        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }
    }

    // C<Mask>(I,J) = T, so C and T must be compatible.
    // also C<Mask>(I,J) = accum(C,T) for entries in T but not C
    if (scalar_expansion)
    {
        if (!GB_Type_code_compatible (C->type->code, scalar_code))
        {
            return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
                "input scalar of type [%s]\n"
                "cannot be typecast to output of type [%s]",
                GB_code_string (scalar_code), C->type->name))) ;
        }
    }
    else
    {
        if (!GB_Type_compatible (C->type, A->type))
        {
            return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
                "input of type [%s]\n"
                "cannot be typecast to output of type [%s]",
                A->type->name, C->type->name))) ;
        }
    }

    // check the mask
    if (row_assign)
    {
        // must be the same size as one row of C
        ASSERT (ni == 1) ;
        ASSERT (!scalar_expansion && !col_assign) ;
        info = GB_Mask_compatible (Mask, NULL, 1, C->ncols) ;
    }
    else if (col_assign)
    {
        // must be the same size as one column of C
        ASSERT (nj == 1) ;
        ASSERT (!scalar_expansion && !row_assign) ;
        info = GB_Mask_compatible (Mask, NULL, C->nrows, 1) ;
    }
    else
    {
        // must be the same size as C for entire matrix (or vector) assignment,
        // where A is either a matrix or a scalar
        info = GB_Mask_compatible (Mask, NULL, C->nrows, C->ncols) ;
    }
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // check the dimensions of A
    //--------------------------------------------------------------------------

    if (!scalar_expansion)
    {
        int64_t anrows = (A_transpose) ? A->ncols : A->nrows ;
        int64_t ancols = (A_transpose) ? A->nrows : A->ncols ;
        anrows = (A_transpose) ? A->ncols : A->nrows ;
        ancols = (A_transpose) ? A->nrows : A->ncols ;
        if (ni != anrows || nj != ancols)
        {
            return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
                "Dimensions not compatible:\n"
                "IxJ is "GBd"-by-"GBd"\n"
                "input is "GBd"-by-"GBd"%s",
                ni, nj, anrows, ancols, A_transpose ?  " (transposed)" : ""))) ;
        }
    }

    //--------------------------------------------------------------------------
    // quick return if an empty Mask is complemented
    //--------------------------------------------------------------------------

    if (Mask_comp && Mask == NULL)
    {
        // The Mask is empty, and complemented, and thus M(i,j)=0 for all i and
        // j.  The result does not depend on A, I, J, or accum.  The output C
        // is either untouched (if C_replace is false) or cleared (if C_replace
        // is true).  However, the GrB_Row_assign and GrB_Col_assign only clear
        // their specific row or column of C, respectively.

        // Mask is NULL so C and Mask cannot be the same, and A is ignored so
        // it doesn't matter whether or not C == A.  Thus C is not aliased
        // to the inputs.

        if (C_replace)
        {
            if (row_assign)
            {
                // delete all entries in row I [0]
                if (PENDING (C))
                {
                    // all pending tuples must first be assembled
                    APPLY_PENDING_UPDATES (C) ;
                }
                const int64_t *Cp = C->p ;
                int64_t *Ci = C->i ;
                int64_t i = I [0] ;
                for (int64_t j = 0 ; j < C->ncols ; j++)
                {
                    // find C(i,j) if it exists
                    int64_t p = Cp [j] ;
                    int64_t pright = Cp [j+1]-1 ;
                    bool found, is_zombie ;
                    GB_BINARY_ZOMBIE (i, Ci, p, pright, found, C->nzombies,
                        is_zombie) ;
                    if (found && !is_zombie)
                    {
                        // delete C(i,j) by marking it as a zombie
                        ASSERT (i == Ci [p]) ;
                        C->nzombies++ ;
                        Ci [p] = FLIP (i) ;
                    }
                }
            }
            else if (col_assign)
            {
                // delete all entries in column J [0]
                if (PENDING (C))
                {
                    // all pending tuples must first be assembled
                    APPLY_PENDING_UPDATES (C) ;
                }
                const int64_t *Cp = C->p ;
                int64_t *Ci = C->i ;
                int64_t j = J [0] ;
                for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
                {
                    int64_t i = Ci [p] ;
                    if (i >= 0)
                    {
                        // delete C(i,j) by marking it as a zombie
                        C->nzombies++ ;
                        Ci [p] = FLIP (i) ;
                    }
                }
            }
            else
            {
                // C<~NULL>=NULL since result does not depend on computing Z.
                // Since C_replace is true, all of C is cleared.  This is the
                // same as the RETURN_IF_QUICK_MASK macro, except that C may
                // have zombies and pending tuples.
                GB_Matrix_clear (C) ;
            }
        }

        if (C->nzombies > 0)
        {
            // make sure C is in the queue
            GB_queue_insert (C) ;
        }
        // finalize C if blocking mode is enabled, and return result
        ASSERT_OK (GB_check (C, "Final C for assign, quick mask", 0)) ;
        return (GB_block (C)) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace for final C_replace phase
    //--------------------------------------------------------------------------

    // IJ_whole_matrix is true if C(:,:)=A is being computed (the submatrix is
    // all of C), or all that the operation can modify for row/col assign.

    bool IJ_whole_matrix ;
    if (row_assign)
    {
        // row assignment to the entire row
        IJ_whole_matrix = (J == GrB_ALL) ;
    }
    else if (col_assign)
    {
        // col assignment to the entire column
        IJ_whole_matrix = (I == GrB_ALL) ;
    }
    else
    {
        // matrix assignment to the entire matrix
        IJ_whole_matrix = (I == GrB_ALL && J == GrB_ALL) ;
    }

    // Mask_is_same is true if SubMask == Mask (:,:)
    bool Mask_is_same = (Mask == NULL || IJ_whole_matrix) ;

    // C_replace_phase is true if a final pass over all of C is required
    // to delete entries outside the C(I,J) submatrix.
    bool C_replace_phase = (C_replace && Mask != NULL && !IJ_whole_matrix) ;

    if (C_replace_phase)
    {
        // Mark must be size C->nrows + C->ncols.  There are cases where it
        // could be smaller, but this ensures all cases have enough space.
        if (!GB_Mark_alloc (C->nrows + C->ncols))
        {
            double memory = GBYTES (C->nrows + C->ncols, sizeof (int64_t)) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
    }

    //--------------------------------------------------------------------------
    // apply pending updates to A and Mask
    //--------------------------------------------------------------------------

    // if C == Mask or C == A, pending updates are applied to C as well

    // delete any lingering zombies and assemble any pending tuples
    // but only in A and Mask, not C
    APPLY_PENDING_UPDATES (Mask) ;
    if (!scalar_expansion)
    {
        APPLY_PENDING_UPDATES (A) ;
    }

    //--------------------------------------------------------------------------
    // initialize workspace
    //--------------------------------------------------------------------------

    GrB_Index *I2 = NULL ;
    GrB_Index *J2 = NULL ;
    GrB_Matrix AT = NULL ;
    GrB_Matrix Mask2 = NULL ;

    //--------------------------------------------------------------------------
    // scalar expansion: sort I and J and remove duplicates
    //--------------------------------------------------------------------------

    if (scalar_expansion)
    {
        // The spec states that scalar expansion is well-defined if I and J
        // have duplicate entries.  However, GxB_subassign is not defined in
        // this case.  To ensure that GrB_assign is well-defined, duplicates in
        // I and J must first be removed.  This reduces the size of I and J,
        // but has no effect on any other parameters.  This can be done in
        // GrB_assign since the Mask has the same size as C (or the entire
        // row/column for GrB_Row_assign and GrB_Col_assign).  It cannot be
        // done in GxB_subassign since its Mask has the same size as IxJ.

        if (I != GrB_ALL && ni > 1)
        {
            info = GB_ijsort (I, &ni, &I2) ;
            if (info != GrB_SUCCESS)
            {
                FREE_ALL ;
                return (info) ;
            }
            I = I2 ;
        }

        if (J != GrB_ALL && nj > 1)
        {
            info = GB_ijsort (J, &nj, &J2) ;
            if (info != GrB_SUCCESS)
            {
                FREE_ALL ;
                return (info) ;
            }
            J = J2 ;
        }
    }

    //--------------------------------------------------------------------------
    // transpose A if requested
    //--------------------------------------------------------------------------

    GrB_Matrix A2 ;

    if (scalar_expansion)
    {
        A2 = NULL ;
    }
    else if (A_transpose)
    {
        // [ AT = A'
        GB_NEW (&AT, A->type, A->ncols, A->nrows, false, true) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            return (info) ;
        }
        info = GB_Matrix_transpose (AT, A, NULL, true) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            return (info) ;
        }
        // AT->p initialized ]
        A2 = AT ;
    }
    else
    {
        A2 = (GrB_Matrix) A ;
    }

    //--------------------------------------------------------------------------
    // extract the SubMask = Mask (I,J) if needed
    //--------------------------------------------------------------------------

    GrB_Matrix SubMask ;

    if (Mask_is_same)
    {
        // the Mask is the same for GrB_assign and GxB_subassign.  Either
        // both masks are NULL, or SubMask = Mask (:,:), and the two masks
        // are equalivalent.
        SubMask = Mask ;
    }
    else
    {
        // extract SubMask = Mask (I,J), keep the same type
        GB_NEW (&Mask2, Mask->type, ni, nj, true, false) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            return (info) ;
        }
        if (row_assign)
        {
            // Mask2 = Mask (0,J) == Mask (:,J)
            info = GB_extract (Mask2,
                false,      // C_replace is false
                NULL,       // no mask
                false,      // mask not complemented
                NULL,       // no accum
                Mask,       // matrix to extract from
                false,      // do not transpose Mask
                GrB_ALL, 0, // row indices (:)
                J, nj) ;    // column indices
        }
        else if (col_assign)
        {
            // Mask2 = Mask (I,0) == Mask (I,:)
            info = GB_extract (Mask2,
                false,      // C_replace is false
                NULL,       // no mask
                false,      // mask not complemented
                NULL,       // no accum
                Mask,       // matrix to extract from
                false,      // do not transpose Mask
                I, ni,      // row indices
                GrB_ALL, 0) ;    // column indices (:)
        }
        else
        {
            // Mask2 = Mask (I,J)
            info = GB_extract (Mask2,
                false,      // C_replace is false
                NULL,       // no mask
                false,      // mask not complemented
                NULL,       // no accum
                Mask,       // matrix to extract from
                false,      // do not transpose Mask
                I, ni,      // row indices
                J, nj) ;    // column indices
        }
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            return (info) ;
        }
        SubMask = Mask2 ;
        ASSERT_OK (GB_check (SubMask, "extracted SubMask", 0)) ;
    }

    //--------------------------------------------------------------------------
    // Z = C
    //--------------------------------------------------------------------------

    // GB_subassign_kernel modifies C efficiently in place, but it can only do
    // so if C is not aliased with A2 or SubMask.  If C is aliased a copy must
    // be made.  GB_subassign_kernel operates on the copy, Z, which is then
    // transplanted back into C when done.  This is costly, and can have
    // performance implications, but it is the only reasonable method.  If C is
    // aliased, then the assignment is a large one and copying the whole matrix
    // will not add much time.

    GrB_Matrix Z ;
    bool aliased = (C == A2 || C == SubMask) ;
    if (aliased)
    {
        // Z = duplicate of C
        info = GB_Matrix_dup (&Z, C) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            return (info) ;
        }
    }
    else
    {
        // GB_subassign_kernel can safely operate on C in place
        Z = C ;
    }

    //--------------------------------------------------------------------------
    // Z(I,J)<SubMask> = A or accum (Z(I,J),A)
    //--------------------------------------------------------------------------

    info = GB_subassign_kernel (
        Z,          C_replace,      // Z matrix and its descriptor
        SubMask,    Mask_comp,      // Mask matrix and its descriptor
        accum,                      // for accum (C(I,J),A)
        A2,                         // A matrix, NULL for scalar expansion
        I, ni,                      // row indices
        J, nj,                      // column indices
        scalar_expansion,           // if true, expand scalar to A
        scalar,                     // scalar to expand, NULL if A not NULL
        scalar_code) ;              // type code of scalar to expand

    // free AT and Mask2
    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&Mask2) ;

    // return if GB_subassign_kernel failed
    if (info != GrB_SUCCESS)
    {
        if (aliased) GB_MATRIX_FREE (&Z) ;
        FREE_ALL ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // examine Z outside the Z(I,J) submatrix
    //--------------------------------------------------------------------------

    if (C_replace_phase)
    {
        // Let M be the mask operator as determined by the Mask matrix.  If
        // C_replace is true and M(i,j)=0 for any entry outside the Z(I,J)
        // submatrix, then that entry must be deleted.  This phase is very
        // costly but it is what the GraphBLAS Specification requires.
        // This phase is skipped if C_replace is false.

        // This case can only occur if the Mask is present (either complemented
        // or not).  If the Mask is not present, then it is not complemented
        // (see the "quick return" case above).  So if there is no Mask
        // matrix, M(I,J)=1 is true, so C_replace has no effect outside the
        // Z(I,J) submatrix.

        // Also, if IJ_whole_matrix is true, then there is nothing outside
        // the Z(I,J) submatrix to modify, so this phase is skipped if
        // IJ_whole_matrix is true.

        // This code assumes Z and Mask are not aliased to each other.

        //----------------------------------------------------------------------
        // assemble any pending tuples
        //----------------------------------------------------------------------

        if (Z->npending > 0)
        {
            info = GB_wait (Z) ;
            if (info != GrB_SUCCESS)
            {
                if (aliased) GB_MATRIX_FREE (&Z) ;
                FREE_ALL ;
                return (info) ;
            }
        }

        //----------------------------------------------------------------------
        // use Mark workspace to flag rows/cols inside the Z(I,J) submatrix
        //----------------------------------------------------------------------

        int64_t flag = GB_Mark_reset (1,0) ;
        int64_t *Mark_col = NULL, *Mark_row = NULL ;
        int64_t *Mark = GB_thread_local.Mark ;

        if (I != GrB_ALL)
        {
            // Mark_row has size Z->nrows
            Mark_row = Mark ;
            Mark += Z->nrows ;
            for (int64_t k = 0 ; k < ni ; k++)
            {
                Mark_row [I [k]] = flag ;
            }
        }

        if (J != GrB_ALL)
        {
            // Mark_col has size Z->ncols
            Mark_col = Mark ;
            for (int64_t k = 0 ; k < nj ; k++)
            {
                Mark_col [J [k]] = flag ;
            }
        }

        //----------------------------------------------------------------------
        // get Z and the Mask
        //----------------------------------------------------------------------

        const int64_t *Zp = Z->p ;
        int64_t *Zi = Z->i ;

        const int64_t *Maskp = Mask->p ;
        const int64_t *Maski = Mask->i ;
        const void *Maskx = Mask->x ;
        size_t msize = Mask->type->size ;
        GB_cast_function cast_Mask_to_bool =
            GB_cast_factory (GB_BOOL_code, Mask->type->code) ;

        //----------------------------------------------------------------------
        // delete entries outside Z(I,J) for which M(i,j) is false
        //----------------------------------------------------------------------

        if (row_assign)
        {

            //------------------------------------------------------------------
            // row assignment, examine just Z(i,:)
            //------------------------------------------------------------------

            // GrB_Row_assign: only examine the row Z(i,:)
            // Mask is a single row
            int64_t i = I [0] ;

            for (int64_t j = 0 ; j < Z->ncols ; j++)
            {
                // j_outside is true if column j is outside the Z(I,J) submatrix
                bool j_outside = (Mark_col != NULL) && (Mark_col [j] < flag) ;

                if (j_outside)
                {
                    // find Z(i,j) if it exists
                    int64_t p = Zp [j] ;
                    int64_t pright = Zp [j+1]-1 ;
                    bool found, is_zombie ;
                    GB_BINARY_ZOMBIE (i, Zi, p, pright, found, Z->nzombies,
                        is_zombie) ;
                    if (found && !is_zombie)
                    {
                        // Z(i,j) is a live entry not in the Z(I,J) submatrix.
                        // Check the Mask(0,j) to see if it should be deleted.
                        bool Mij = false ;
                        int64_t pmask = Maskp [j] ;
                        if (pmask < Maskp [j+1])
                        {
                            // found it
                            cast_Mask_to_bool (&Mij, Maskx +(pmask*msize), 0) ;
                        }
                        if (Mask_comp)
                        {
                            // negate the Mask if Mask_comp is true
                            Mij = !Mij ;
                        }
                        if (Mij == false)
                        {
                            // delete Z(i,j) by marking it as a zombie
                            Z->nzombies++ ;
                            Zi [p] = FLIP (i) ;
                        }
                    }
                }
            }

        }
        else if (col_assign)
        {

            //------------------------------------------------------------------
            // column assignment, examine just Z(:,j)
            //------------------------------------------------------------------

            // Mask is a single column
            int64_t j = J [0] ;

            for (int64_t p = Zp [j] ; p < Zp [j+1] ; p++)
            {
                // Z(i,j) is outside the Z(I,j) subcolumn if either i is
                // not in the list I
                int64_t i = Zi [p] ;
                if (i < 0)
                {
                    // Z(i,j) is already a zombie; skip it.
                    continue ;
                }
                bool i_outside = (Mark_row != NULL) && (Mark_row [i] < flag) ;

                if (i_outside)
                {
                    // Z(i,j) is a live entry not in the Z(I,j) subcolumn.
                    // Check the Mask to see if it should be deleted.
                    bool Mij ;
                    int64_t pleft  = Maskp [0] ;
                    int64_t pright = Maskp [1] - 1 ;
                    bool found ;
                    GB_BINARY_SEARCH (i, Maski, pleft, pright, found) ;
                    if (found)
                    {
                        // found it
                        cast_Mask_to_bool (&Mij, Maskx +(pleft*msize), 0) ;
                    }
                    else
                    {
                        // Mask(i,j) not present, implicitly false
                        Mij = false ;
                    }
                    if (Mask_comp)
                    {
                        // negate the Mask if Mask_comp is true
                        Mij = !Mij ;
                    }
                    if (Mij == false)
                    {
                        // delete Z(i,j) by marking it as a zombie
                        Z->nzombies++ ;
                        Zi [p] = FLIP (i) ;
                    }
                }
            }
        }
        else
        {

            //------------------------------------------------------------------
            // Matrix/vector assignment: examine all of Z
            //------------------------------------------------------------------

            // Mask has the same size as Z
            for (int64_t j = 0 ; j < Z->ncols ; j++)
            {
                // j_outside is true if column j is outside the Z(I,J) submatrix
                bool j_outside = (Mark_col != NULL) && (Mark_col [j] < flag) ;

                for (int64_t p = Zp [j] ; p < Zp [j+1] ; p++)
                {
                    // Z(i,j) is outside the Z(I,J) submatrix if either i is
                    // not in the list I, or j is not in J, or both.
                    int64_t i = Zi [p] ;
                    if (i < 0)
                    {
                        // Z(i,j) is already a zombie; skip it.
                        continue ;
                    }
                    bool i_outside = (Mark_row != NULL)
                                  && (Mark_row [i] < flag) ;

                    if (j_outside || i_outside)
                    {
                        // Z(i,j) is a live entry not in the Z(I,J) submatrix.
                        // Check the Mask to see if it should be deleted.
                        bool Mij ;
                        int64_t pleft  = Maskp [j] ;
                        int64_t pright = Maskp [j+1] - 1 ;
                        bool found ;
                        GB_BINARY_SEARCH (i, Maski, pleft, pright, found) ;
                        if (found)
                        {
                            // found it
                            cast_Mask_to_bool (&Mij, Maskx +(pleft*msize), 0) ;
                        }
                        else
                        {
                            // Mask(i,j) not present, implicitly false
                            Mij = false ;
                        }
                        if (Mask_comp)
                        {
                            // negate the Mask if Mask_comp is true
                            Mij = !Mij ;
                        }
                        if (Mij == false)
                        {
                            // delete Z(i,j) by marking it as a zombie
                            Z->nzombies++ ;
                            Zi [p] = FLIP (i) ;
                        }
                    }
                }
            }
        }

        // clear the Mark array
        GB_Mark_reset (1,0) ;
    }

    // free workspace
    FREE_ALL ;

    //--------------------------------------------------------------------------
    // C = Z
    //--------------------------------------------------------------------------

    if (aliased)
    {
        // zombies can be transplanted into C but pending tuples cannot
        if (Z->npending > 0)
        {
            // assemble all pending tuples, and delete all zombies too
            info = GB_wait (Z) ;
        }
        if (info == GrB_SUCCESS)
        {
            // transplants the content of Z into C and frees Z.
            // this always succeeds since nothing gets allocated.
            info = GB_Matrix_transplant (C, C->type, &Z) ;
            ASSERT (info == GrB_SUCCESS) ;
        }
        if (info != GrB_SUCCESS)
        {
            // Z needs to be freed if C is aliased but info != GrB_SUCCESS.
            // C remains unchanged.
            GB_MATRIX_FREE (&Z) ;
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // cleanup
    //--------------------------------------------------------------------------

    if (C->nzombies > 0)
    {
        // make sure C is in the queue
        GB_queue_insert (C) ;
    }

    // finalize C if blocking mode is enabled, and return result
    ASSERT_OK (GB_check (C, "Final C for assign", 0)) ;
    return (GB_block (C)) ;
}

#undef FREE_ALL


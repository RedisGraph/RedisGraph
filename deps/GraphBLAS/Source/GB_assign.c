//------------------------------------------------------------------------------
// GB_assign: submatrix assignment: C<M>(Rows,Cols) = accum (C(Rows,Cols),A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// submatrix assignment: C<M>(Rows,Cols) = accum (C(Rows,Cols),A)

// All GrB_*_assign operations rely on this function.

// Only one of the bool parameters: scalar_expansion, col_assign, and
// row_assign can be true.  If all are false, this function does the work for
// GrB_Matrix_assign and GrB_Vector_assign.

// If scalar_expansion is true, this function performs scalar assignment (the
// GrB_Matrix_assign_TYPE and GrB_Vector_assign_TYPE functions) in which case
// the input matrix A is ignored (it is NULL), and the scalar is used instead.

// If col_assign is true, this function does the work for GrB_Col_assign.
// If row_assign is true, this function does the work for GrB_Row_assign.

// Compare with GB_subassign, which uses M and C_replace differently

#include "GB_assign.h"
#include "GB_subassign.h"
#include "GB_subref.h"
#include "GB_transpose.h"

#define GB_FREE_ALL                                     \
{                                                       \
    GB_MATRIX_FREE (&Z2) ;                              \
    GB_MATRIX_FREE (&AT) ;                              \
    GB_MATRIX_FREE (&MT) ;                              \
    GB_FREE_MEMORY (I2,  I2_size, sizeof (GrB_Index)) ; \
    GB_FREE_MEMORY (I2k, I2_size, sizeof (GrB_Index)) ; \
    GB_FREE_MEMORY (J2,  J2_size, sizeof (GrB_Index)) ; \
    GB_FREE_MEMORY (J2k, J2_size, sizeof (GrB_Index)) ; \
    GB_MATRIX_FREE (&SubMask) ;                         \
}

GrB_Info GB_assign                  // C<M>(Rows,Cols) += A or A'
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C
    const bool Mask_comp,           // true if mask is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    bool M_transpose,               // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    bool A_transpose,               // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    const bool col_assign,          // true for GrB_Col_assign
    const bool row_assign,          // true for GrB_Row_assign
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix AT = NULL ;
    GrB_Matrix MT = NULL ;
    GrB_Matrix Z = NULL ;
    GrB_Matrix Z2 = NULL ;
    GrB_Index *GB_RESTRICT I2  = NULL ;
    GrB_Index *GB_RESTRICT I2k = NULL ;
    GrB_Index *GB_RESTRICT J2  = NULL ;
    GrB_Index *GB_RESTRICT J2k = NULL ;
    int64_t I2_size = 0, J2_size = 0 ;
    GrB_Matrix SubMask = NULL ;

    // C may be aliased with M_in and/or A_in

    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL (Rows) ;
    GB_RETURN_IF_NULL (Cols) ;

    GrB_Matrix M = M_in ;
    GrB_Matrix A = A_in ;

    if (scalar_expansion)
    { 
        // for scalar expansion, the NULL pointer case has been already checked
        // for user-defined types, and can't be NULL for built-in types.
        ASSERT (scalar != NULL) ;
        ASSERT (A == NULL) ;
        ASSERT (!row_assign && !col_assign) ;
    }
    else
    { 
        // GrB_*assign, not scalar:  The user's input matrix has been checked.
        // The pointer to the scalar is NULL.
        ASSERT (scalar == NULL) ;
        ASSERT_MATRIX_OK (A, "A for GB_assign", GB0) ;
    }

    ASSERT_MATRIX_OK (C, "C input for GB_assign", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_assign", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_assign", GB0) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    // only one of these three cases can be true:
    ASSERT (row_assign + col_assign + scalar_expansion <= 1) ;

    // row_assign always uses M_transpose == true.
    // all other methods use M_transpose == false.
    ASSERT (row_assign == M_transpose) ;

    int64_t nRows, nCols, RowColon [3], ColColon [3] ;
    int RowsKind, ColsKind ;
    GB_ijlength (Rows, nRows_in, GB_NROWS (C), &nRows, &RowsKind, RowColon) ;
    GB_ijlength (Cols, nCols_in, GB_NCOLS (C), &nCols, &ColsKind, ColColon) ;

    bool whole_C_matrix = (RowsKind == GB_ALL && ColsKind == GB_ALL) ;

    bool C_is_csc = C->is_csc ;

    //--------------------------------------------------------------------------
    // check domains and dimensions for C<M>(Rows,Cols) += A or A'
    //--------------------------------------------------------------------------

    // GB_compatible is not used since most of it is slightly different here
    if (accum != NULL)
    { 
        // C<M>(Rows,Cols) = accum (C(Rows,Cols),A)
        GB_OK (GB_BinaryOp_compatible (accum, C->type, C->type,
            (scalar_expansion) ? NULL : A->type,
            (scalar_expansion) ? scalar_code : GB_ignore_code, Context)) ;
    }

    // C<M>(Rows,Cols) = T, so C and T must be compatible.
    // also C<M>(Rows,Cols) = accum(C,T) for entries in T but not C
    if (scalar_expansion)
    {
        if (!GB_code_compatible (C->type->code, scalar_code))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "input scalar of type [%s]\n"
                "cannot be typecast to output of type [%s]",
                GB_code_string (scalar_code), C->type->name))) ;
        }
    }
    else
    {
        if (!GB_Type_compatible (C->type, A->type))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "input of type [%s]\n"
                "cannot be typecast to output of type [%s]",
                A->type->name, C->type->name))) ;
        }
    }

    // check the dimensions and type of M
    if (M != NULL)
    {
        // M is typecast to boolean
        if (!GB_Type_compatible (M->type, GrB_BOOL))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "M of type [%s] cannot be typecast to boolean",
                M->type->name))) ;
        }
        // check the mask: size depends on the method
        if (row_assign)
        {
            // GrB_Row_assign:
            // M is a column vector the same size as one row of C
            ASSERT (nRows == 1) ;
            ASSERT (!scalar_expansion && !col_assign) ;
            ASSERT (GB_VECTOR_OK (M)) ;
            if (GB_NROWS (M) != GB_NCOLS (C))
            { 
                return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                    "mask vector m length is "GBd"; must match the number of "
                    "columns of C ("GBd")", GB_NROWS (M), GB_NCOLS (C)))) ;
            }
        }
        else if (col_assign)
        {
            // GrB_Col_assign:
            // M is a column vector the same size as one column of C
            ASSERT (nCols == 1) ;
            ASSERT (!scalar_expansion && !row_assign) ;
            ASSERT (GB_VECTOR_OK (M)) ;
            if (GB_NROWS (M) != GB_NROWS (C))
            { 
                return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                    "mask vector m length is "GBd"; must match the number of "
                    "rows of C ("GBd")", GB_NROWS (M), GB_NROWS (C)))) ;
            }
        }
        else
        {
            // GrB_Matrix_assign, GrB_Vector_assign, and scalar variants:
            // M is a matrix the same size as C for entire matrix (or vector)
            // assignment, where A is either a matrix or a scalar
            if (GB_NROWS (M) != GB_NROWS (C) || GB_NCOLS (M) != GB_NCOLS (C))
            { 
                return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                    "mask M is "GBd"-by-"GBd"; "
                    "must match result C ("GBd"-by-"GBd")",
                    GB_NROWS (M), GB_NCOLS (M), GB_NROWS (C), GB_NCOLS (C)))) ;
            }
        }
    }

    // check the dimensions of A
    if (!scalar_expansion)
    {
        int64_t anrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
        int64_t ancols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
        if (nRows != anrows || nCols != ancols)
        { 
            return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                "Dimensions not compatible:\n"
                "C(Rows,Cols) is "GBd"-by-"GBd"\n"
                "input is "GBd"-by-"GBd"%s",
                nRows, nCols, anrows, ancols,
                A_transpose ? " (transposed)" : ""))) ;
        }
    }

    //--------------------------------------------------------------------------
    // quick return if an empty mask is complemented
    //--------------------------------------------------------------------------

    if (Mask_comp && M == NULL)
    {
        // The mask M is empty, and complemented, and thus M(i,j)=0 for all i
        // and j.  The result does not depend on A, Rows, Cols, or accum.  The
        // output C is either untouched (if C_replace is false) or cleared (if
        // C_replace is true).  However, the GrB_Row_assign and GrB_Col_assign
        // only clear their specific row or column of C, respectively.

        // M is NULL so C and M cannot be the same, and A is ignored so
        // it doesn't matter whether or not C == A.  Thus C is not aliased
        // to the inputs.

        if (C_replace)
        {
            ASSERT_MATRIX_OK (C, "C for quick mask", GB0) ;
            if (row_assign || col_assign)
            {
                // all pending tuples must first be assembled; zombies OK
                GB_WAIT_PENDING (C) ;
                ASSERT_MATRIX_OK (C, "waited C for quick mask", GB0) ;
                if ((row_assign && !C_is_csc) || (col_assign && C_is_csc))
                { 
                    // delete all entries in vector j
                    GBBURBLE ("C(:,j)=zombie ") ;
                    int64_t j = (col_assign) ? Cols [0] : Rows [0] ;
                    GB_assign_zombie1 (C, j, Context) ;
                }
                else
                { 
                    // delete all entries in each vector with index i
                    GBBURBLE ("C(i,:)=zombie ") ;
                    int64_t i = (row_assign) ? Rows [0] : Cols [0] ;
                    GB_assign_zombie2 (C, i, Context) ;
                }
            }
            else
            { 
                // C<!NULL>=NULL since result does not depend on computing Z.
                // Since C_replace is true, all of C is cleared.  This is the
                // same as the GB_RETURN_IF_QUICK_MASK macro, except that C may
                // have zombies and pending tuples.
                return (GB_clear (C, Context)) ;
            }
        }

        if (C->nzombies > 0)
        { 
            // make sure C is in the queue
            GB_CRITICAL (GB_queue_insert (C)) ;
        }
        // finalize C if blocking mode is enabled, and return result
        ASSERT_MATRIX_OK (C, "Final C for assign, quick mask", GB0) ;
        return (GB_block (C, Context)) ;
    }

    //--------------------------------------------------------------------------
    // determine if the final C_replace phase is needed
    //--------------------------------------------------------------------------

    // whole_submatrix is true if C(:,:)=A is being computed (the submatrix is
    // all of C), or all that the operation can modify for row/col assign.

    bool whole_submatrix ;
    if (row_assign)
    { 
        // row assignment to the entire row
        whole_submatrix = (ColsKind == GB_ALL) ;
    }
    else if (col_assign)
    { 
        // col assignment to the entire column
        whole_submatrix = (RowsKind == GB_ALL) ;
    }
    else
    { 
        // matrix assignment to the entire matrix
        whole_submatrix = whole_C_matrix ;
    }

    // Mask_is_same is true if SubMask == M (:,:)
    bool Mask_is_same = (M == NULL || whole_submatrix) ;

    // C_replace_phase is true if a final pass over all of C is required
    // to delete entries outside the C(I,J) submatrix.
    bool C_replace_phase = (C_replace && !Mask_is_same) ;
    ASSERT (!Mask_is_same == (M != NULL && !whole_submatrix)) ;

    //--------------------------------------------------------------------------
    // apply pending updates to A and M
    //--------------------------------------------------------------------------

    // if C == M or C == A, pending updates are applied to C as well

    // delete any lingering zombies and assemble any pending tuples
    // but only in A and M, not C
    GB_WAIT (M) ;
    if (!scalar_expansion)
    { 
        GB_WAIT (A) ;
    }

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format of C:
    //--------------------------------------------------------------------------

    // GrB_Row_assign: A is always a vector in CSC format, and A_transpose is
    // always true.  If C is in CSC format then A_transpose remains true, and
    // the n-by-1 vector A is transposed below into a 1-by-n hypersparse CSC
    // matrix.  If C is in CSR format then A_transpose becomes false, and the
    // assignment does not need to transpose A.  It remains in CSC format but
    // has the correct vector length and dimension for the CSR/CSC-agnostic
    // assignment.

    // GrB_Col_assign: A is always a vector in CSC format, and A_transpose is
    // always false.  If C is in CSC format then A_transpose remains false, and
    // the assignment does not need to transpose A.  If C is in CSR format then
    // A_transpose becomes true, and the the n-by-1 vector A is transposed
    // below into a 1-by-n hypersparse CSC matrix.  The CSC format is ignored
    // by the CSR/CSC-agnostic assignment.

    // GrB_Vector_assign:  both A and C are always in CSC format, and
    // A_transpose is always false, and doesn't change below.

    // GrB_Matrix_assign:  A and C can be in any format, and A_transpose can be
    // true or false, depending on the descriptor.  If the CSR/CSC formats of A
    // and C are the same, then A_transpose remains as-is.  If they differ,
    // then A_transpose is flipped.  Then the CSR-CSC agnostic assignment
    // proceeds.

    if (!scalar_expansion && C_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    // get the I and J index lists
    int Ikind, Jkind ;
    const GrB_Index *I, *J ;
    int64_t ni, nj, nI, nJ, *Icolon, *Jcolon ;

    if (C_is_csc)
    { 
        // C is in CSC format
        I      = Rows     ;     J      = Cols     ;
        ni     = nRows_in ;     nj     = nCols_in ;
        Ikind  = RowsKind ;     Jkind  = ColsKind ;
        nI     = nRows    ;     nJ     = nCols    ;
        Icolon = RowColon ;     Jcolon = ColColon ;
    }
    else
    { 
        // C is in CSR format
        I       = Cols     ;    J       = Rows     ;
        ni      = nCols_in ;    nj      = nRows_in ;
        Ikind   = ColsKind ;    Jkind   = RowsKind ;
        nI      = nCols    ;    nJ      = nRows    ;
        Icolon  = ColColon ;    Jcolon  = RowColon ;
    }

    // C has cnvec vectors, where cnvec <= C->vdim
    // J is a list of vectors in the range 0:C->vdim-1
    // I is a list of indices in the range 0:C->vlen-1

    //--------------------------------------------------------------------------
    // scalar expansion: sort I and J and remove duplicates
    //--------------------------------------------------------------------------

    if (scalar_expansion)
    {
        // The spec states that scalar expansion is well-defined if I and J
        // have duplicate entries.  However, GB_subassigner is not defined in
        // this case.  To ensure that GrB_assign is well-defined, duplicates in
        // I and J must first be removed.  This reduces the size of I and J,
        // but has no effect on any other parameters.  This can be done here
        // since the mask M has the same size as C (or the entire row/column
        // for GrB_Row_assign and GrB_Col_assign).  It cannot be done in
        // GB_subassigner since its mask has the same size as IxJ.

        // no need to sort a list of length 0 or 1; it is already sorted

        if (Ikind == GB_LIST && ni > 1)
        { 
            // ni and nI are reduced if there are duplicates
            I2_size = ni ;
            GB_OK (GB_ijsort (I, &ni, &I2, &I2k, Context)) ;
            ASSERT (ni <= I2_size) ;
            nI = ni ;
            I = I2 ;
        }

        if (Jkind == GB_LIST && nj > 1)
        { 
            // nj and nJ are reduced if there are duplicates
            J2_size = nj ;
            GB_OK (GB_ijsort (J, &nj, &J2, &J2k, Context)) ;
            ASSERT (nj <= J2_size) ;
            nJ = nj ;
            J = J2 ;
        }
    }

    //--------------------------------------------------------------------------
    // transpose A if requested
    //--------------------------------------------------------------------------

    // GrB_Row_assign and GrB_Col_assign pass A as a typecasted vector,
    // which is then quickly transposed to a hypersparse matrix.

    if (!scalar_expansion && A_transpose)
    { 
        // AT = A', with no typecasting
        // transpose: no typecast, no op, not in place
        GBBURBLE ("(A transpose) ") ;
        GB_OK (GB_transpose (&AT, NULL, C_is_csc, A, NULL, Context)) ;
        A = AT ;
    }

    //--------------------------------------------------------------------------
    // extract the SubMask = M (I,J) if needed
    //--------------------------------------------------------------------------

    if (Mask_is_same)
    { 
        // the mask M is the same for GB_assign and GB_subassign.  Either
        // both masks are NULL, or SubMask = M (:,:), and the two masks
        // are equivalent.
        ASSERT_MATRIX_OK_OR_NULL (SubMask, "SubMask is same as M", GB0) ;
    }
    else
    {
        // extract M (I,J)
        ASSERT_MATRIX_OK (M, "big mask", GB0) ;
        if (row_assign)
        {
            // SubMask = M(Cols,:), but use I or J if they are the sorted I2, J2
            ASSERT (GB_VECTOR_OK (M)) ;
            ASSERT (M->is_csc) ;
            if (C_is_csc)
            { 
                // SubMask = Mask (J,:)
                ASSERT (J == J2 || J == Cols) ;
                GB_OK (GB_subref (&SubMask, true, M,
                    J, nj, GrB_ALL, 1, false, true, Context)) ;
            }
            else
            { 
                // SubMask = Mask (I,:)
                ASSERT (I == I2 || I == Cols) ;
                GB_OK (GB_subref (&SubMask, true, M,
                    I, ni, GrB_ALL, 1, false, true, Context)) ;
            }
            ASSERT (GB_VECTOR_OK (SubMask)) ;
        }
        else if (col_assign)
        {
            // SubMask = M(Rows,:), but use I or J if they are the sorted I2, J2
            ASSERT (GB_VECTOR_OK (M)) ;
            ASSERT (M->is_csc) ;
            if (C_is_csc)
            { 
                // SubMask = Mask (I,:)
                ASSERT (I == I2 || I == Rows) ;
                GB_OK (GB_subref (&SubMask, true, M,
                    I, ni, GrB_ALL, 1, false, true, Context)) ;
            }
            else
            { 
                // SubMask = Mask (J,:)
                ASSERT (J == J2 || J == Rows) ;
                GB_OK (GB_subref (&SubMask, true, M,
                    J, nj, GrB_ALL, 1, false, true, Context)) ;
            }
            ASSERT (GB_VECTOR_OK (SubMask)) ;
        }
        else
        {
            // SubMask = M (I,J)
            if (M->is_csc == C_is_csc)
            { 
                GB_OK (GB_subref (&SubMask, M->is_csc,
                    M, I, ni, J, nj, false, true, Context)) ;
            }
            else
            { 
                GB_OK (GB_subref (&SubMask, M->is_csc,
                    M, J, nj, I, ni, false, true, Context)) ;
            }
        }
        M = SubMask ;
        ASSERT_MATRIX_OK (M, "extracted submask M", GB0) ;
    }

    //--------------------------------------------------------------------------
    // transpose the mask if requested
    //--------------------------------------------------------------------------

    // the mask for G*B_Col_*assign and G*B_Row_*assign is a GrB_Vector in CSC
    // form, which is quickly transposed to a hypersparse matrix, if needed.
    // G*B_Vector_*assign always has a CSC mask and CSC C matrix, since both
    // are GrB_Vectors.

    if (M != NULL)
    {
        if (M->is_csc != C_is_csc)
        { 
            // either G*B_Row_*assign and G*B_Col_*assign when matrix C is in
            // CSR format, and or G*B_Matrix_assign when the format of the
            // matrices C and M differ.
            M_transpose = !M_transpose ;
        }
        if (M_transpose)
        { 
            // MT = M' to conform M to the same CSR/CSC format as C.
            // typecast to boolean, if a full matrix transpose is done.
            // transpose: typecast, no op, not in place
            GBBURBLE ("(M transpose) ") ;
            GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M, NULL, Context)) ;
            M = MT ;
        }
    }

    //--------------------------------------------------------------------------
    // make a copy Z = C if C is aliased to A or M
    //--------------------------------------------------------------------------

    // If C is aliased to A and/or M, a copy must be made.  GB_subassigner
    // operates on the copy, Z, which is then transplanted back into C when
    // done.  This is costly, and can have performance implications, but it is
    // the only reasonable method.  If a copy of C must be made, then it is as
    // large as M or A, so copying the whole matrix will not add much time.

    bool C_aliased = GB_aliased (C, A) || GB_aliased (C, M) ;

    // GB_assign cannot tolerate any alias with the input mask,
    // if the C_replace phase will be performed.
    if (C_replace_phase)
    { 
        // the C_replace_phase requires C and M_in not to be aliased
        C_aliased = C_aliased || GB_aliased (C, M_in) ;
    }

    if (C_aliased)
    {
        // If C is aliased, it no longer has any pending work, A and M have
        // been finished, above.  This also ensures GB_dup does not need to
        // finish any pending work in C.
        GBBURBLE ("(C aliased) ") ;
        ASSERT (!GB_ZOMBIES (C)) ;
        ASSERT (!GB_PENDING (C)) ;
        if (whole_C_matrix && C_replace && accum == NULL)
        { 
            // C(:,:)<any mask, replace> = A or x, with C aliased to M or A.  C
            // is about to be cleared in GB_subassigner anyway, but a duplicate
            // is needed because C is aliased with M or A.  Instead of
            // duplicating it, create an empty matrix Z2.  This also prevents
            // the C_replace_phase from being needed.
            GB_NEW (&Z2, C->type, C->vlen, C->vdim, GB_Ap_calloc, C->is_csc,
                GB_SAME_HYPER_AS (C->is_hyper), C->hyper_ratio, 1, Context) ;
            GB_OK (info)  ;
            GBBURBLE ("(C alias cleared; C_replace early) ") ;
            C_replace = false ;
            C_replace_phase = false ;
        }
        else
        { 
            // Z2 = duplicate of C, which must be freed when done
            GB_OK (GB_dup (&Z2, C, true, NULL, Context)) ;
        }
        Z = Z2 ;
    }
    else
    {
        // GB_subassigner can safely operate on C in place and so can the
        // C_replace_phase below.
        // FUTURE:  if C is dense and will remain so,
        // it would be faster to delay the clearing of C.
        if (whole_C_matrix && C_replace && accum == NULL)
        { 
            // C(:,:)<any mask, replace> = A or x, with C not aliased to M or
            // A.  C is about to be cleared in GB_subassigner anyway, so clear
            // it now.  This also prevents the C_replace_phase from being
            // needed.
            GB_OK (GB_clear (C, Context)) ;
            GBBURBLE ("(C(:,:)<any mask>: C_replace early) ") ;
            C_replace = false ;
            C_replace_phase = false ;
        }
        Z = C ;
    }

    //--------------------------------------------------------------------------
    // Z(I,J)<M> = A or accum (Z(I,J),A)
    //--------------------------------------------------------------------------

    GB_OK (GB_subassigner (
        Z,          C_replace,      // Z matrix and its descriptor
        M, Mask_comp, Mask_struct,  // mask matrix and its descriptor
        accum,                      // for accum (C(I,J),A)
        A,                          // A matrix, NULL for scalar expansion
        I, ni,                      // indices
        J, nj,                      // vectors
        scalar_expansion,           // if true, expand scalar to A
        scalar,                     // scalar to expand, NULL if A not NULL
        scalar_code,                // type code of scalar to expand
        Context)) ;

    // free all but Z2, I2, and J2, which are still needed.  MT and SubMask are
    // freed because the C_replace_phase requires the original mask, not the
    // submask or its transpose.  Z2 is still needed since Z == Z2, and it will
    // be modified by the C_replace_phase and then transplanted back into C.
    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&MT) ;
    GB_MATRIX_FREE (&SubMask) ;

    //--------------------------------------------------------------------------
    // examine Z outside the Z(I,J) submatrix
    //--------------------------------------------------------------------------

    if (C_replace_phase)
    {
        // If C_replace is true and M_in(i,j)=0 for any entry outside the
        // Z(I,J) submatrix, then that entry must be deleted.  This phase is
        // very costly but it is what the GraphBLAS Specification requires.
        // This phase is skipped if C_replace is false.

        // This case can only occur if the mask is present (either complemented
        // or not).  If the mask is not present, then it is not complemented
        // (see the "quick return" case above).  So if there is no mask matrix,
        // M_in(I,J)=1 is true, so C_replace has no effect outside the Z(I,J)
        // submatrix.

        // Also, if whole_submatrix is true, then there is nothing outside the
        // Z(I,J) submatrix to modify, so this phase is skipped if
        // whole_submatrix is true.

        // This code requires Z and M_in not to be aliased to each other.

        M = M_in ;
        ASSERT (M != NULL) ;
        ASSERT (!GB_aliased (Z, M)) ;

        ASSERT_MATRIX_OK (Z, "Z for C-replace-phase", GB0) ;
        ASSERT_MATRIX_OK (M, "M for C-replace-phase", GB0) ;

        //----------------------------------------------------------------------
        // assemble any pending tuples
        //----------------------------------------------------------------------

        if (GB_PENDING (Z))
        { 
            GB_OK (GB_wait (Z, Context)) ;
        }

        ASSERT_MATRIX_OK (Z, "Z cleaned up for C-replace-phase", GB0) ;

        //----------------------------------------------------------------------
        // get the original mask and transpose it if required
        //----------------------------------------------------------------------

        // M is the now all of M_in, not the SubMask, so it must be transposed

        if (M_transpose)
        { 
            // MT = M' to conform M to the same CSR/CSC format as C.
            // typecast to boolean, if a full matrix transpose is done.
            // transpose: typecast, no op, not in place
            GBBURBLE ("(M transpose) ") ;
            GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M, NULL, Context)) ;
            M = MT ;
        }

        ASSERT_MATRIX_OK (M, "M transposed for C-replace-phase", GB0) ;

        //----------------------------------------------------------------------
        // sort I and J, if they are GB_LIST, if not already done
        //----------------------------------------------------------------------

        // I2 and J2 may already have been constructed for scalar expansion,
        // so allocate them and construct them, if needed.

        ASSERT (GB_IMPLIES (I2 != NULL, (I == I2) && (I2_size > 1))) ;
        ASSERT (GB_IMPLIES (J2 != NULL, (J == J2) && (J2_size > 1))) ;

        if (Ikind == GB_LIST && ni > 1 && I2 == NULL)
        { 
            // ni and nI are reduced if there are duplicates
            I2_size = ni ;
            GB_OK (GB_ijsort (I, &ni, &I2, &I2k, Context)) ;
            ASSERT (ni <= I2_size) ;
            nI = ni ;
            I = I2 ;
        }

        if (Jkind == GB_LIST && nj > 1 && J2 == NULL)
        { 
            // nj and nJ are reduced if there are duplicates
            J2_size = nj ;
            GB_OK (GB_ijsort (J, &nj, &J2, &J2k, Context)) ;
            ASSERT (nj <= J2_size) ;
            nJ = nj ;
            J = J2 ;
        }

        //----------------------------------------------------------------------
        // delete entries outside Z(I,J) for which M(i,j) is false
        //----------------------------------------------------------------------

        if ((row_assign && !C->is_csc) || (col_assign && C->is_csc))
        { 

            //------------------------------------------------------------------
            // vector assignment, examine all of M but just Z(:,j)
            //------------------------------------------------------------------

            // M is a single column so it is never hypersparse
            ASSERT (nJ == 1) ;
            ASSERT (M->vlen == Z->vlen && M->vdim == 1 && !M->is_hyper) ;
            ASSERT (Jkind == GB_LIST) ;
            int64_t j = J [0] ;
            ASSERT (j == GB_ijlist (J, 0, Jkind, Jcolon)) ;

            GBBURBLE ("assign zombies outside C(I,j) ") ;
            GB_assign_zombie3 (Z, M, Mask_comp, Mask_struct,
                j, I, nI, Ikind, Icolon, Context) ;
        }
        else if ((row_assign && C->is_csc) || (col_assign && !C->is_csc))
        { 

            //------------------------------------------------------------------
            // index assignment, examine just Z(i,:) and M
            //------------------------------------------------------------------

            // GrB_Row_assign: only examine Z(i,:)
            // M has vlen == 1 and the same vdim as Z
            ASSERT (nI == 1) ;
            ASSERT (M->vlen == 1 && M->vdim == Z->vdim) ;
            ASSERT (Ikind == GB_LIST) ;
            int64_t i = I [0] ;
            ASSERT (i == GB_ijlist (I, 0, Ikind, Icolon)) ;

            GBBURBLE ("assign zombies outside C(i,J) ") ;
            GB_assign_zombie4 (Z, M, Mask_comp, Mask_struct,
                i, J, nJ, Jkind, Jcolon, Context) ;
        }
        else
        { 

            //------------------------------------------------------------------
            // Matrix/vector assignment: examine all of Z and M
            //------------------------------------------------------------------

            // M has the same size as Z
            ASSERT (M->vlen == Z->vlen && M->vdim == Z->vdim) ;

            GBBURBLE ("assign zombies outside C(I,J) ") ;
            GB_OK (GB_assign_zombie5 (Z, M, Mask_comp, Mask_struct,
                I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon, Context)) ;
        }

        // Z is valid, but it has zombies and it not in the queue.
        ASSERT_MATRIX_OK (Z, "Z for C-replace-phase done", GB_FLIP (GB0)) ;
    }

    //--------------------------------------------------------------------------
    // transplant Z2 back into C
    //--------------------------------------------------------------------------

    if (C_aliased)
    {
        // zombies can be transplanted into C but pending tuples cannot
        if (GB_PENDING (Z2))
        { 
            // assemble all pending tuples, and delete all zombies too
            GB_OK (GB_wait (Z2, Context)) ;
        }
        // transplants the content of Z into C and frees Z
        GB_OK (GB_transplant (C, C->type, &Z2, Context)) ;
    }

    // The hypersparsity of C is not modified.  This will be done eventually,
    // when all pending operations are completed via GB_wait.

    //--------------------------------------------------------------------------
    // free workspace, finalize C, and return result
    //--------------------------------------------------------------------------

    if (C->nzombies > 0)
    { 
        // make sure C is in the queue.  GB_subassigner can place it in the
        // queue, but it might not need to if the matrix has no zombies or
        // pending tuples.  Zombies can be added by the C_replace_phase.
        GB_CRITICAL (GB_queue_insert (C)) ;
    }

    // finalize C if blocking mode is enabled, and return result

    ASSERT_MATRIX_OK (C, "Final C for assign", GB0) ;
    GB_FREE_ALL ;
    return (GB_block (C, Context)) ;
}


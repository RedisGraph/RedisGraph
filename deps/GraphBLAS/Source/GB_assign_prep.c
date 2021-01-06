//------------------------------------------------------------------------------
// GB_assign_prep: check and prepare inputs for GB_assign and GB_subassign
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_assign_prep checks the inputs for all assignment methods:
// GrB_Row_assign, GrB_Col_assign, GrB_assign, and GxB_subassign.

#include "GB_subassign.h"
#include "GB_bitmap_assign.h"
#include "GB_assign_zombie.h"
#include "GB_subassign_methods.h"
#include "GB_transpose.h"
#include "GB_subref.h"

#undef  GB_FREE_ALL
#define GB_FREE_ALL             \
{                               \
    GB_Matrix_free (&C2) ;      \
    GB_Matrix_free (&A2) ;      \
    GB_Matrix_free (&AT) ;      \
    GB_Matrix_free (&M2) ;      \
    GB_Matrix_free (&MT) ;      \
    GB_FREE (I2) ;              \
    GB_FREE (J2) ;              \
    GB_FREE (I2k) ;             \
    GB_FREE (J2k) ;             \
}

GrB_Info GB_assign_prep
(
    // output:
    GrB_Matrix *Chandle,            // C_in, or C2 if C is aliased to M or A
    GrB_Matrix *Mhandle,            // M_in, or a modified version M2
    GrB_Matrix *Ahandle,            // A_in, or a modified version A2

    // modified versions of the matrices C, M, and A:
    GrB_Matrix *C2_handle,          // NULL, or a copy of C
    GrB_Matrix *M2_handle,          // NULL, or a temporary matrix
    GrB_Matrix *A2_handle,          // NULL, or a temporary matrix

    // modified versions of the Rows/Cols lists, and their analysis:
    const GrB_Index **I_handle,     // Rows, Cols, or a modified copy I2
    GrB_Index **I2_handle,          // NULL, or sorted/pruned Rows or Cols
    int64_t *ni_handle,
    int64_t *nI_handle,
    int *Ikind_handle,
    int64_t Icolon [3],

    const GrB_Index **J_handle,     // Rows, Cols, or a modified copy J2
    GrB_Index **J2_handle,          // NULL, or sorted/pruned Rows or Cols
    int64_t *nj_handle,
    int64_t *nJ_handle,
    int *Jkind_handle,
    int64_t Jcolon [3],

    bool *done,                     // true if the prep has finished all work
    GrB_Type *atype_handle,         // type of A or the scalar

    // input/output
    GrB_Matrix C_in,                // input/output matrix for results
    bool *C_replace,                // descriptor for C
    int *assign_kind,               // row/col assign, assign, or subassign

    // input
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
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;
    GB_RETURN_IF_NULL (Rows) ;
    GB_RETURN_IF_NULL (Cols) ;

    GrB_Matrix C = C_in ;
    GrB_Matrix M = M_in ;
    GrB_Matrix A = A_in ;

    ASSERT_MATRIX_OK (C, "C input for GB_assign/subassign", GB0) ;
    ASSERT (!GB_is_shallow (C)) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_assign/subassign", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_assign/subassign", GB0) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    GrB_Matrix C2 = NULL ;
    GrB_Matrix M2 = NULL ;
    GrB_Matrix A2 = NULL ;
    GrB_Matrix MT = NULL ;
    GrB_Matrix AT = NULL ;

    GrB_Index *I2 = NULL ;
    GrB_Index *J2 = NULL ;
    GrB_Index *I2k = NULL ;
    GrB_Index *J2k = NULL ;
    (*done) = false ;
    (*atype_handle) = NULL ;

    (*Chandle) = NULL ;
    (*Mhandle) = NULL ;
    (*Ahandle) = NULL ;

    (*C2_handle) = NULL ;
    (*A2_handle) = NULL ;
    (*M2_handle) = NULL ;

    (*I_handle) = NULL ; 
    (*I2_handle) = NULL ;
    (*ni_handle) = 0 ;
    (*nI_handle) = 0 ;
    (*Ikind_handle) = 0 ;

    (*J_handle) = NULL ;
    (*J2_handle) = NULL ;
    (*nj_handle) = 0 ;
    (*nJ_handle) = 0 ;
    (*Jkind_handle) = 0 ;

    //--------------------------------------------------------------------------
    // determine the type of A or the scalar
    //--------------------------------------------------------------------------

    GrB_Type atype ;
    if (scalar_expansion)
    { 
        // for scalar expansion, the NULL pointer case has been already checked
        // for user-defined types, and can't be NULL for built-in types.
        ASSERT (scalar != NULL) ;
        ASSERT (A == NULL) ;
        ASSERT ((*assign_kind) == GB_ASSIGN || (*assign_kind) == GB_SUBASSIGN) ;
        atype = GB_code_type (scalar_code, C->type) ;
    }
    else
    { 
        // GrB_*assign, not scalar:  The user's input matrix has been checked.
        // The pointer to the scalar is NULL.
        ASSERT (scalar == NULL) ;
        ASSERT_MATRIX_OK (A, "A for GB_assign/GB_subassign", GB0) ;
        atype = A->type ;
    }

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    // zombies and pending tuples in C or OK, but not M or A
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (M) ;
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (A) ;

    // some kernels allow for M and A to be jumbled
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;

    // C can have any kind of pending work
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (GB_PENDING_OK (C)) ;

    //--------------------------------------------------------------------------
    // check domains of C, M, A, and accum
    //--------------------------------------------------------------------------

    // GB_compatible is not used since most of it is slightly different here
    if (accum != NULL)
    { 
        // C<M>(Rows,Cols) = accum (C(Rows,Cols),A), or
        // C(Rows,Cols)<M> = accum (C(Rows,Cols),A)
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
            GB_ERROR (GrB_DOMAIN_MISMATCH, "Input scalar of type [%s]\n"
                "cannot be typecast to output of type [%s]",
                GB_code_string (scalar_code), C->type->name) ;
        }
    }
    else
    {
        if (!GB_Type_compatible (C->type, A->type))
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH, "Input of type [%s]\n"
                "cannot be typecast to output of type [%s]",
                A->type->name, C->type->name) ;
        }
    }

    if (M != NULL)
    {
        // M is typecast to boolean
        if (!GB_Type_compatible (M->type, GrB_BOOL))
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "M of type [%s] cannot be typecast to boolean", M->type->name) ;
        }
    }

    //--------------------------------------------------------------------------
    // determine the properites of the Rows and Cols index lists
    //--------------------------------------------------------------------------

    int64_t nRows, nCols, RowColon [3], ColColon [3] ;
    int RowsKind, ColsKind ;
    GB_ijlength (Rows, nRows_in, GB_NROWS (C), &nRows, &RowsKind, RowColon) ;
    GB_ijlength (Cols, nCols_in, GB_NCOLS (C), &nCols, &ColsKind, ColColon) ;

    //--------------------------------------------------------------------------
    // check the dimensions of M
    //--------------------------------------------------------------------------

    if (M != NULL)
    {
        // check the mask: size depends on the method

        switch (*assign_kind)
        {
            case GB_ROW_ASSIGN : 
            {
                // GrB_Row_assign:
                // M is a column vector the same size as one row of C
                ASSERT (nRows == 1) ;
                ASSERT (!scalar_expansion) ;
                ASSERT (GB_VECTOR_OK (M)) ;
                if (GB_NROWS (M) != GB_NCOLS (C))
                { 
                    GB_ERROR (GrB_DIMENSION_MISMATCH, "Mask vector m length"
                        " is " GBd "; must match the number of columns of C ("
                        GBd ")", GB_NROWS (M), GB_NCOLS (C)) ;
                }
            }
            break ;

            case GB_COL_ASSIGN : 
            {
                // GrB_Col_assign:
                // M is a column vector the same size as one column of C
                ASSERT (nCols == 1) ;
                ASSERT (!scalar_expansion) ;
                ASSERT (GB_VECTOR_OK (M)) ;
                if (GB_NROWS (M) != GB_NROWS (C))
                { 
                    GB_ERROR (GrB_DIMENSION_MISMATCH, "Mask vector m length"
                        " is " GBd "; must match the number of rows of C ("
                        GBd ")", GB_NROWS (M), GB_NROWS (C)) ;
                }
            }
            break ;

            case GB_ASSIGN : 
            {
                // GrB_Matrix_assign, GrB_Vector_assign, and scalar variants: M
                // is a matrix the same size as C for entire matrix (or vector)
                // assignment, where A is either a matrix or a scalar
                if (GB_NROWS (M) != GB_NROWS (C) ||
                    GB_NCOLS (M) != GB_NCOLS (C))
                { 
                    GB_ERROR (GrB_DIMENSION_MISMATCH, "Mask M is " GBd "-by-"
                        GBd "; " "must match result C (" GBd "-by-" GBd ")",
                        GB_NROWS (M), GB_NCOLS (M),
                        GB_NROWS (C), GB_NCOLS (C)) ;
                }
            }
            break ;

            case GB_SUBASSIGN : 
            {
                // GxB_subassign: M is a matrix the same size as C(Rows,Cols)
                int64_t mnrows = M_transpose ? GB_NCOLS (M) : GB_NROWS (M) ;
                int64_t mncols = M_transpose ? GB_NROWS (M) : GB_NCOLS (M) ;
                if (mnrows != nRows || mncols != nCols)
                { 
                    GB_ERROR (GrB_DIMENSION_MISMATCH,
                        "M is " GBd "-by-" GBd "%s, "
                        "must match size of result C(I,J): " GBd "-by-" GBd "",
                        mnrows, mncols, M_transpose ? " (transposed)" : "",
                        nRows, nCols) ;
                }
            }
            break ;

            default:
                ASSERT (GB_DEAD_CODE) ;
        }
    }

    //--------------------------------------------------------------------------
    // check the dimensions of A
    //--------------------------------------------------------------------------

    if (!scalar_expansion)
    {
        int64_t anrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
        int64_t ancols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
        if (nRows != anrows || nCols != ancols)
        { 
            GB_ERROR (GrB_DIMENSION_MISMATCH,
                "Dimensions not compatible:\n"
                "C(Rows,Cols) is " GBd "-by-" GBd "\n"
                "input is " GBd "-by-" GBd "%s",
                nRows, nCols, anrows, ancols,
                A_transpose ? " (transposed)" : "") ;
        }
    }

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format of C:
    //--------------------------------------------------------------------------

    // GrB_Row_assign, GxB_Row_subassign: A is always a vector in CSC format,
    // and A_transpose is always true.  If C is in CSC format then A_transpose
    // remains true, and the n-by-1 vector A is transposed below into a 1-by-n
    // hypersparse CSC matrix.  If C is in CSR format then A_transpose becomes
    // false, and the assignment does not need to transpose A.  It remains in
    // CSC format but has the correct vector length and dimension for the
    // CSR/CSC-agnostic assignment.

    // GrB_Col_assign, GxB_Col_subassign: A is always a vector in CSC format,
    // and A_transpose is always false.  If C is in CSC format then A_transpose
    // remains false, and the assignment does not need to transpose A.  If C is
    // in CSR format then A_transpose becomes true, and the the n-by-1 vector A
    // is transposed below into a 1-by-n hypersparse CSC matrix.  The CSC
    // format is ignored by the CSR/CSC-agnostic assignment.

    // GrB_Vector_assign, GxB_Vector_subassign:  both A and C are always in CSC
    // format, and A_transpose is always false, and doesn't change below.

    // GrB_Matrix_assign, GxB_Matrix_subassign:  A and C can be in any format,
    // and A_transpose can be true or false, depending on the descriptor.  If
    // the CSR/CSC formats of A and C are the same, then A_transpose remains
    // as-is.  If they differ, then A_transpose is flipped.  Then the CSR-CSC
    // agnostic assignment proceeds.

    bool C_is_csc = C->is_csc ;
    if (!scalar_expansion && C_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    // get the I and J index lists
    int Ikind, Jkind ;
    const GrB_Index *I, *J ;
    int64_t ni, nj, nI, nJ ;

    if (C_is_csc)
    { 
        // C is in CSC format
        I      = Rows     ;     J      = Cols     ;
        ni     = nRows_in ;     nj     = nCols_in ;
        Ikind  = RowsKind ;     Jkind  = ColsKind ;
        nI     = nRows    ;     nJ     = nCols    ;
        memcpy (Icolon, RowColon, 3 * sizeof (int64_t)) ;
        memcpy (Jcolon, ColColon, 3 * sizeof (int64_t)) ;
    }
    else
    { 
        // C is in CSR format
        I       = Cols     ;    J       = Rows     ;
        ni      = nCols_in ;    nj      = nRows_in ;
        Ikind   = ColsKind ;    Jkind   = RowsKind ;
        nI      = nCols    ;    nJ      = nRows    ;
        memcpy (Icolon, ColColon, 3 * sizeof (int64_t)) ;
        memcpy (Jcolon, RowColon, 3 * sizeof (int64_t)) ;
        // flip the sense of row/col assign
        if ((*assign_kind) == GB_ROW_ASSIGN)
        {
            // assignment to vector j = J [0], which is Rows [0]
            (*assign_kind) = GB_COL_ASSIGN ;
        }
        else if ((*assign_kind) == GB_COL_ASSIGN)
        {
            // assignment to index i = I [0], which is Cols [0]
            (*assign_kind) = GB_ROW_ASSIGN ;
        }
    }

    // J is now a list of vectors in the range 0:C->vdim-1
    // I is now a list of indices in the range 0:C->vlen-1

    bool whole_C_matrix = (Ikind == GB_ALL && Jkind == GB_ALL) ;

    //--------------------------------------------------------------------------
    // quick return if an empty mask is complemented
    //--------------------------------------------------------------------------

    bool C_is_bitmap = GB_IS_BITMAP (C) ;
    int C_sparsity = GB_sparsity_control (C->sparsity, C->vdim) ;
    bool C_may_be_bitmap = (C_sparsity & GxB_BITMAP) ;
    bool use_bitmap_assign = (C_is_bitmap ||
        ((*C_replace) && GB_IS_FULL (C) && C_may_be_bitmap)) ;

    // an empty mask occurs when M is not present, but complemented

    if (M == NULL && Mask_comp)
    {

        //----------------------------------------------------------------------
        // C<!,replace or !replace>(I,J) = anything
        //----------------------------------------------------------------------

        // The mask M is empty, and complemented, and thus M(i,j)=0 for all i
        // and j.  The result does not depend on A or accum.  The output C is
        // either untouched (if C_replace is false) or cleared (if C_replace is
        // true).  However, the GrB_Row_assign and GrB_Col_assign only clear
        // their specific row or column of C, respectively.  GB_subassign only
        // clears C(I,J).  GrB_assign clears all of C.

        // M is NULL so C and M cannot be the same, and A is ignored so
        // it doesn't matter whether or not C == A.  Thus C is not aliased
        // to the inputs.

        // This condition is like GB_RETURN_IF_QUICK_MASK(...), except that
        // the action taken by C_replace is different for row/col assign
        // and subassign.

        if (*C_replace)
        {

            //------------------------------------------------------------------
            // C<!,replace>(I,J) = anything
            //------------------------------------------------------------------

            ASSERT_MATRIX_OK (C, "C for quick mask", GB0) ;

            // to clear the whole C matrix: assign and subassign are the same

            switch (whole_C_matrix ? GB_ASSIGN : (*assign_kind))
            {

                //--------------------------------------------------------------
                // row assign: delete all entries in C(i,:)
                //--------------------------------------------------------------

                case GB_ROW_ASSIGN : 
                {
                    // delete all entries in each vector with index i
                    GB_MATRIX_WAIT_IF_PENDING (C) ;
                    if (use_bitmap_assign)
                    { 
                        // neither A nor the scalar are used, so convert this
                        // to a scalar assignment (the scalar is not used)
                        GBURBLE ("bitmap C(i,:)=zombie ") ;
                        int scalar_unused = 0 ;
                        GB_OK (GB_bitmap_assign (C, /* C_replace: */ true,
                            I,    1, GB_LIST, NULL, // I
                            NULL, 0, GB_ALL,  NULL, // J
                            /* no M: */ NULL,
                            /* Mask_comp: */ true,
                            /* Mask_struct: ignored */ false,
                            /* no accum: */ NULL,
                            /* no A: */ NULL,
                            /* scalar: */ &scalar_unused, GrB_INT32,
                            GB_ROW_ASSIGN, Context)) ;
                    }
                    else
                    { 
                        GB_MATRIX_WAIT_IF_JUMBLED (C) ;
                        GB_ENSURE_SPARSE (C) ;
                        GBURBLE ("C(i,:)=zombie ") ;
                        GB_assign_zombie2 (C, I [0], Context) ;
                    }
                }
                break ;

                //--------------------------------------------------------------
                // col assign: delete all entries in C(:,j)
                //--------------------------------------------------------------

                case GB_COL_ASSIGN : 
                {
                    GB_MATRIX_WAIT_IF_PENDING (C) ;
                    if (use_bitmap_assign)
                    { 
                        // neither A nor the scalar are used, so convert this
                        // to a scalar assignment (the scalar is not used)
                        GBURBLE ("bitmap C(:,j)=zombie ") ;
                        int scalar_unused = 0 ;
                        GB_OK (GB_bitmap_assign (C, /* C_replace: */ true,
                            NULL, 0, GB_ALL,  NULL, // I
                            J,    1, GB_LIST, NULL, // J
                            /* no M: */ NULL,
                            /* Mask_comp: */ true,
                            /* Mask_struct: ignored */ false,
                            /* no accum: */ NULL,
                            /* no A: */ NULL,
                            /* scalar: */ &scalar_unused, GrB_INT32,
                            GB_COL_ASSIGN, Context)) ;
                    }
                    else
                    { 
                        GB_ENSURE_SPARSE (C) ;
                        GBURBLE ("C(:,j)=zombie ") ;
                        GB_assign_zombie1 (C, J [0], Context) ;
                    }
                }
                break ;

                //--------------------------------------------------------------
                // assign: delete all entries in C
                //--------------------------------------------------------------

                case GB_ASSIGN : 
                {
                    // C<!>=anything since result does not depend on computing
                    // Z.  Since C_replace is true, all of C is cleared.  This
                    // is the same as the GB_RETURN_IF_QUICK_MASK macro.
                    // GB_clear either converts C to an empty sparse/hyper
                    // matrix, or to a bitmap matrix with no entries, depending
                    // on its sparsity control setting.
                    GBURBLE ("clear C ") ;
                    GB_OK (GB_clear (C, Context)) ;
                }
                break ;

                //--------------------------------------------------------------
                // subassign: delete all entries in C(I,J)
                //--------------------------------------------------------------

                case GB_SUBASSIGN : 
                {
                    GB_MATRIX_WAIT_IF_PENDING (C) ;
                    if (use_bitmap_assign)
                    { 
                        // neither A nor the scalar are used, so convert this
                        // to a scalar assignment (the scalar is not used)
                        GBURBLE ("bitmap C(I,J)=zombie ") ;
                        int scalar_unused = 0 ;
                        GB_OK (GB_bitmap_assign (C, /* C_replace: */ true,
                            I, nI, Ikind, Icolon,
                            J, nJ, Jkind, Jcolon,
                            /* no M: */ NULL,
                            /* Mask_comp: */ true,
                            /* Mask_struct: ignored */ false,
                            /* no accum: */ NULL,
                            /* no A: */ NULL,
                            /* scalar: */ &scalar_unused, GrB_INT32,
                            GB_SUBASSIGN, Context)) ;
                    }
                    else
                    { 
                        // Method 00: C(I,J) = empty, using S
                        GBURBLE ("C(I,J)=zombie ") ;
                        GB_ENSURE_SPARSE (C) ;
                        GB_OK (GB_subassign_zombie (C,
                            I, ni, nI, Ikind, Icolon,
                            J, nj, nJ, Jkind, Jcolon, Context)) ;
                    }
                }
                break ;

                default: ;
            }
        }

        //----------------------------------------------------------------------
        // finalize C if blocking mode is enabled, and return result
        //----------------------------------------------------------------------

        ASSERT_MATRIX_OK (C, "Final C for assign, quick mask", GB0) ;
        (*done) = true ;
        GB_FREE_ALL ;
        ASSERT (C == C_in) ;
        (*Chandle) = C ;
        return (GB_block (C, Context)) ;
    }

    //--------------------------------------------------------------------------
    // disable C_replace if no mask present
    //--------------------------------------------------------------------------

    bool no_mask = (M == NULL && !Mask_comp) ;
    if (no_mask)
    {
        // no_mask:  mask is not present, and not complemented
        if (*C_replace)
        { 
            // The mask is not present and not complemented.  In this case,
            // C_replace is effectively false for subassign.  Disable it, since
            // it can force pending tuples to be assembled.
            GBURBLE ("(no mask: C_replace effectively false) ") ;
            (*C_replace) = false ;
        }
    }

    //--------------------------------------------------------------------------
    // delete pending tuples for C(:,:) = x and C(:,:) = A
    //--------------------------------------------------------------------------

    if (whole_C_matrix)
    { 
        // If the assignment is C<M>(:,:) = ... then convert the assignment
        // into a subassign.
        (*assign_kind) = GB_SUBASSIGN ;
    }

    if (whole_C_matrix && no_mask && accum == NULL)
    { 

        //----------------------------------------------------------------------
        // C(:,:) = x or A:  whole matrix assignment with no mask
        //----------------------------------------------------------------------

        // C_replace is already effectively false (see no_mask condition above)
        ASSERT ((*C_replace) == false) ;

        if (GB_aliased (C, A) && !A_transpose && !scalar_expansion)
        { 
            // C = C, with C and A aliased, no transpose, no mask, no accum
            // operator, both I and J are ":", Mask_comp false.  C is not
            // modified at all, and there's no work to do except to check for
            // blocking mode.
            GBURBLE ("(no-op) ") ;
            (*done) = true ;
            GB_FREE_ALL ;
            ASSERT (C == C_in) ;
            (*Chandle) = C ;
            return (GB_block (C, Context)) ;
        }

        // free pending tuples early but do not clear C.  If it is
        // already dense then its pattern can be reused.
        GB_Pending_free (&(C->Pending)) ;
    }

    //--------------------------------------------------------------------------
    // transpose A if requested
    //--------------------------------------------------------------------------

    // GrB_Row_assign and GrB_Col_assign pass A as a typecasted vector,
    // which is then quickly transposed to a hypersparse matrix.

    ASSERT_MATRIX_OK (C, "C here in GB_assign", GB0) ;

    if (!scalar_expansion && A_transpose)
    { 
        // AT = A', with no typecasting
        // transpose: no typecast, no op, not in-place
        // TODO: if accum is present and it does not depend on the values of
        // A,  only construct the pattern of AT, not the values.
        GBURBLE ("(A transpose) ") ;
        GB_OK (GB_transpose (&AT, NULL, C_is_csc, A,
            NULL, NULL, NULL, false, Context)) ;
        GB_MATRIX_WAIT (AT) ;       // A cannot be jumbled
        A = AT ;
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
            // MT = M' to conform M to the same CSR/CSC format as C,
            // and typecast to boolean.
            // transpose: typecast, no op, not in-place
            // TODO: if Mask_struct, only construct the pattern of MT
            GBURBLE ("(M transpose) ") ;
            GB_OK (GB_transpose (&MT, GrB_BOOL, C_is_csc, M,
                NULL, NULL, NULL, false, Context)) ;
            GB_MATRIX_WAIT (MT) ;       // M cannot be jumbled
            M = MT ;
        }
    }

    //--------------------------------------------------------------------------
    // determine the properties of I and J
    //--------------------------------------------------------------------------

    // If the descriptor says that A must be transposed, it has already been
    // transposed in the caller.  Thus C(I,J), A, and M (if present) all
    // have the same size: length(I)-by-length(J)

    bool I_unsorted, I_has_dupl, I_contig, J_unsorted, J_has_dupl, J_contig ;
    int64_t imin, imax, jmin, jmax ;
    GB_OK (GB_ijproperties (I, ni, nI, C->vlen, &Ikind, Icolon,
                &I_unsorted, &I_has_dupl, &I_contig, &imin, &imax, Context)) ;
    GB_OK (GB_ijproperties (J, nj, nJ, C->vdim, &Jkind, Jcolon,
                &J_unsorted, &J_has_dupl, &J_contig, &jmin, &jmax, Context)) ;

    //--------------------------------------------------------------------------
    // sort I and J and remove duplicates, if needed
    //--------------------------------------------------------------------------

    // If I or J are explicit lists, and either of are unsorted or are sorted
    // but have duplicate entries, then both I and J are sorted and their
    // duplicates are removed.  A and M are adjusted accordingly.  Removing
    // duplicates decreases the length of I and J.

    bool I_unsorted_or_has_dupl = (I_unsorted || I_has_dupl) ;
    bool J_unsorted_or_has_dupl = (J_unsorted || J_has_dupl) ;
    bool presort = I_unsorted_or_has_dupl || J_unsorted_or_has_dupl ;

    // This pre-sort of I and J is required for the parallel assignment.
    // Otherwise, multiple threads may attempt to modify the same part of C.
    // This could cause a race condition, if one thread flags a zombie at the
    // same time another thread is using that index in a binary search.  If the
    // 2nd thread finds either zombie/not-zombie, this is fine, but the
    // modification would have to be atomic.  Atomic read/write is slow, so to
    // avoid the use of atomics, the index lists I and J are sorted and all
    // duplicates are removed.

    // A side benefit of this pre-sort is that it ensures that the results of
    // GrB_assign and GxB_subassign are completely defined if I and J have
    // duplicates.  The definition of this pre-sort is given in MATLAB below.

    /*
        function C = subassign (C, I, J, A)
        % submatrix assignment with pre-sort of I and J; and remove duplicates

        % delete duplicates from I, keeping the last one seen
        [I2 I2k] = sort (I) ;
        Idupl = [(I2 (1:end-1) == I2 (2:end)), false] ;
        I2  = I2  (~Idupl) ;
        I2k = I2k (~Idupl) ;
        assert (isequal (I2, unique (I)))

        % delete duplicates from J, keeping the last one seen
        [J2 J2k] = sort (J) ;
        Jdupl = [(J2 (1:end-1) == J2 (2:end)), false] ;
        J2  = J2  (~Jdupl) ;
        J2k = J2k (~Jdupl) ;
        assert (isequal (J2, unique (J)))

        % do the submatrix assignment, with no duplicates in I2 or J2
        C (I2,J2) = A (I2k,J2k) ;
    */

    // With this subassign script, the result returned by GB_subassigner
    // matches the behavior in MATLAB, so the following holds:

    /*
        C4 = C ;
        C4 (I,J) = A ;
        C3 = subassign (C, I, J, A) ;
        assert (isequal (C4, C3)) ;
    */

    // That is, the pre-sort of I, J, and A has no effect on the final C, in
    // MATLAB.

    // The pre-sort itself takes additional work and memory space, but it may
    // actually improve the performance since it makes the data access of C
    // more regular, even in the sequential case.

    if (presort)
    {

        ASSERT (Ikind == GB_LIST || Jkind == GB_LIST) ;
        ASSERT (!whole_C_matrix) ;

        if (I_unsorted_or_has_dupl)
        { 
            // I2 = sort I and remove duplicates
            ASSERT (Ikind == GB_LIST) ;
            GB_OK (GB_ijsort (I, &ni, &I2, &I2k, Context)) ;
            // Recheck the length and properties of the new I2.  This may
            // convert I2 to GB_ALL or GB_RANGE, after I2 has been sorted.
            GB_ijlength (I2, ni, C->vlen, &nI, &Ikind, Icolon) ;
            GB_OK (GB_ijproperties (I2, ni, nI, C->vlen, &Ikind, Icolon,
                &I_unsorted, &I_has_dupl, &I_contig, &imin, &imax, Context)) ;
            ASSERT (! (I_unsorted || I_has_dupl)) ;
            I = I2 ;
        }

        if (J_unsorted_or_has_dupl)
        { 
            // J2 = sort J and remove duplicates
            ASSERT (Jkind == GB_LIST) ;
            GB_OK (GB_ijsort (J, &nj, &J2, &J2k, Context)) ;
            // Recheck the length and properties of the new J2.  This may
            // convert J2 to GB_ALL or GB_RANGE, after J2 has been sorted.
            GB_ijlength (J2, nj, C->vdim, &nJ, &Jkind, Jcolon) ;
            GB_OK (GB_ijproperties (J2, nj, nJ, C->vdim, &Jkind, Jcolon,
                &J_unsorted, &J_has_dupl, &J_contig, &jmin, &jmax, Context)) ;
            ASSERT (! (J_unsorted || J_has_dupl)) ;
            J = J2 ;
        }

        // inverse index lists to create the A2 and M2 submatrices:
        const GrB_Index *Iinv = I_unsorted_or_has_dupl ? I2k : GrB_ALL ;
        const GrB_Index *Jinv = J_unsorted_or_has_dupl ? J2k : GrB_ALL ;

        if (!scalar_expansion)
        { 
            // A2 = A (Iinv, Jinv)
            GB_OK (GB_subref (&A2, A->is_csc, A, Iinv, ni, Jinv, nj, false,
                Context)) ;
            // GB_subref can return a jumbled result
            ASSERT (GB_JUMBLED_OK (A2)) ;
            if (A == AT) GB_Matrix_free (&AT) ;
            A = A2 ;
        }

        if (M != NULL && (*assign_kind) == GB_SUBASSIGN)
        { 
            // M2 = M (Iinv, Jinv)
            GB_OK (GB_subref (&M2, M->is_csc, M, Iinv, ni, Jinv, nj, false,
                Context)) ;
            // GB_subref can return a jumbled result
            ASSERT (GB_JUMBLED_OK (M2)) ;
            if (M == MT) GB_Matrix_free (&MT) ;
            M = M2 ;
        }

        GB_FREE (I2k) ;
        GB_FREE (J2k) ;
    }

    // I and J are now sorted, with no duplicate entries.  They are either
    // GB_ALL, GB_RANGE, or GB_STRIDE, which are intrinsically sorted with no
    // duplicates, or they are explicit GB_LISTs with sorted entries and no
    // duplicates.

    ASSERT (!I_unsorted) ; ASSERT (!I_has_dupl) ;
    ASSERT (!J_unsorted) ; ASSERT (!J_has_dupl) ;

    //--------------------------------------------------------------------------
    // check for early C_replace
    //--------------------------------------------------------------------------

    // C(:,:)<any mask, replace> = A or x

    // C_replace_may_be_done_early is true if the C_replace action can take
    // place now.  If true, the final C does not depend on the contents of
    // C on input.  If bitmap assigment might be done, delay the clearing of
    // C since it would be faster to set its bitmap to all zero later on,
    // instead of freeing it and reallocating it.

    bool C_replace_may_be_done_early = (whole_C_matrix && (*C_replace)
        && accum == NULL && !use_bitmap_assign) ;

    // If the entire C(:,:) is being assigned to, and if no accum operator is
    // present, then the matrix can be cleared of all entries now, and then
    // C_replace can be set false.  Clearing C now speeds up the assignment
    // since the wait on C can be skipped, below.  It also simplifies the
    // kernels.  If S is constructed, it is just an empty matrix.

    // By clearing C now and setting C_replace to false, the following methods
    // are used: 09 becomes 05, 10 becomes 06n or 06s, 17 becomes 13, and 18
    // becomes 14.  The S matrix for methods 06s, 13, and 14 is still created,
    // but it is very fast to construct and traverse since C is empty.  Method
    // 00 can be skipped since C is already empty (see "quick" case below).

        // prior time             new  time           action
        // ----- ----             ---  ----           ------

        // 00:  O(S)              nothing, O(1)       C already cleared

        // 09:  O(M+S)            05:  O(M)           C<M> = x, no S

        // 10:  O((A+S)*log(m))   06n: O(M*(log(a))   C<M> = A, no S
        //                        06s: O(A*(log(m))   C<M> = A, with S

        // 17:  O(m*n)            13:  O(m*n)         C<!M> = x, with S

        // 18:  O(A*log(m))       14:  O(A*log(m))    C<!M> = A, with S

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============

        //  M   -   -   -   -   -       05:  C(I,J)<M> = x, no S
        //  M   -   -   -   A   -       06n: C(I,J)<M> = A, no S
        //  M   -   -   -   A   S       06s: C(I,J)<M> = A, with S

        //  M   -   r   -   -   S       09:  C(I,J)<M,repl> = x, with S
        //  M   -   r   -   A   S       10:  C(I,J)<M,repl> = A, with S

        //  M   c   -   -   -   S       13:  C(I,J)<!M> = x, with S
        //  M   c   -   -   A   S       14:  C(I,J)<!M> = A, with S

        //  M   c   r   -   -   S       17:  C(I,J)<!M,repl> = x, with S
        //  M   c   r   -   A   S       18:  C(I,J)<!M,repl> = A, with S

        // Methods 09, 10, 17, and 18 are now used only if C(I,J) is a
        // submatrix of C, and not for the whole_C_matrix case.

    //--------------------------------------------------------------------------
    // make a copy Z = C if C is aliased to A or M
    //--------------------------------------------------------------------------

    // TODO: bitmap assign can handle C==M and C==A aliasing in some cases

    // If C is aliased to A and/or M, a copy of C must be made.
    bool C_aliased = GB_aliased (C, A) || GB_aliased (C, M) ;

    // GB_assign cannot tolerate any alias with the input mask,
    // if the C_replace phase will be performed.
    if ((*C_replace) && ((*assign_kind) != GB_SUBASSIGN))
    { 
        // the C_replace phase requires C and M_in not to be aliased
        C_aliased = C_aliased || GB_aliased (C, M_in) ;
    }

    if (C_aliased)
    {
        // C is aliased with M or A: make a copy of C to assign into
        if (C_replace_may_be_done_early)
        { 
            // Instead of duplicating C, create a new empty matrix C2.
            int sparsity = (C->h != NULL) ? GxB_HYPERSPARSE : GxB_SPARSE ;
            GB_OK (GB_new (&C2, // sparse or hyper, new header
                C->type, C->vlen, C->vdim, GB_Ap_calloc, C_is_csc,
                sparsity, C->hyper_switch, 1, Context)) ;
            GBURBLE ("(C alias cleared; C_replace early) ") ;
            (*C_replace) = false ;
        }
        else
        { 
            // finish any computations in C, but leave it jumbled
            GBURBLE ("(C alias: make duplicate) ") ;
            GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (C) ;
            ASSERT (!GB_ZOMBIES (C)) ;
            ASSERT (GB_JUMBLED_OK (C)) ;
            ASSERT (!GB_PENDING (C)) ;
            // C2 = duplicate of C, which must be freed when done
            GB_OK (GB_dup (&C2, C, true, NULL, Context)) ;
        }
        // C2 must be transplanted back into C when done
        C = C2 ;
    }
    else
    {
        // C is not aliased, but check if it can be cleared early
        if (C_replace_may_be_done_early)
        { 
            // Clear C early.
            GB_OK (GB_clear (C, Context)) ;
            GBURBLE ("(C(:,:)<any mask>: C_replace early) ") ;
            (*C_replace) = false ;
        }
        // the assignment operates on C in-place
    }

    //--------------------------------------------------------------------------
    // disable C_replace if C is empty
    //--------------------------------------------------------------------------

    bool C_is_empty = (GB_NNZ (C) == 0 && !GB_PENDING (C) && !GB_ZOMBIES (C)) ;
    if (C_is_empty)
    { 
        // C is completely empty.  C_replace is irrelevant so set it to false.
        (*C_replace) = false ;
    }

    //--------------------------------------------------------------------------
    // check compatibilty of prior pending tuples
    //--------------------------------------------------------------------------

    // The action: ( delete ) can only delete a live entry in the pattern.  It
    // cannot delete a pending tuple; pending tuples cannot become zombies.
    // Thus, if this assignment has the potential for creating zombies, all
    // prior pending tuples must be assembled now.  They thus become live
    // entries in the pattern of C, so that this GB_subassigner can
    // (potentially) turn them into zombies via action: ( delete ).

    // If accum is NULL, the operation is C(I,J) = A, or C(I,J)<M> = A.  If A
    // has any implicit zeros at all, or if M is present, then the
    // action: ( delete ) is possible.  This action is taken when an entry is
    // found in C but not A.  It is thus not possible to check A in advance if
    // an entry in C must be deleted.  If an entry does not appear in C but
    // appears as a pending tuple, deleting it would require a scan of all the
    // pending tuples in C.  This is costly, and simply assembling all pending
    // tuples first is faster.

    // The action: ( insert ) adds additional pending tuples.  All pending
    // tuples will be assembled sometime later on, using a single pending
    // operator, and thus the current accum operator must match the prior
    // pending operator.  If the operators do not match, then all prior pending
    // tuples must be assembled now, so that this GB_subassigner can
    // (potentially) insert new pending tuples whose pending operator is accum.

    // These tests are conservative because it is possible that this
    // GxB_subassign will not need to use action: ( insert ).

    // In the discussion below, let SECOND_Ctype denote the SECOND operator
    // z=f(x,y) whose ztype, xtype, and ytype matches the type of C.

    bool wait = false ;

    if (C->Pending == NULL)
    { 

        //----------------------------------------------------------------------
        // no pending tuples currently exist
        //----------------------------------------------------------------------

        // If any new pending tuples are added, their pending operator is
        // accum, or the implicit SECOND_Ctype operator if accum is NULL.
        // The type of any pending tuples will become C->type.
        // Prior zombies have no effect on this decision.

        wait = false ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // prior pending tuples exist: check if action: ( delete ) can occur
        //----------------------------------------------------------------------

        // action: ( delete ) can only operate on entries in the pattern by
        // turning them into zombies.  It cannot delete prior pending tuples.
        // Thus all prior pending tuples must be assembled first if
        // action: ( delete ) can occur.

        if (*C_replace)
        { 
            // C_replace must use the action: ( delete )
            wait = true ;
        }
        else if (accum == NULL)
        { 
            // This GxB_subassign can potentially use action: ( delete ), and
            // thus prior pending tuples must be assembled first.  However, if
            // A is completely dense, then C(I,J)=A cannot delete any entries
            // from C.

            if (scalar_expansion || GB_is_dense (A))
            { 
                // A is a scalar or dense matrix, so entries cannot be deleted
                wait = false ;
            }
            else
            { 
                // A is sparse.  action: ( delete ) might occur.
                wait = true ;
            }
        }

        //----------------------------------------------------------------------
        // check if pending operator is compatible
        //----------------------------------------------------------------------

        if (!wait)
        { 

            // ( delete ) will not occur, but new pending tuples may be added
            // via the action: ( insert ).  Check if the accum operator is the
            // same as the prior pending operator and ensure the types are
            // the same.

            ASSERT (C->Pending != NULL) ;
            ASSERT (C->Pending->type != NULL) ;

            if (atype != C->Pending->type)
            { 
                // entries in A are copied directly into the list of pending
                // tuples for C, with no typecasting.  The type of the prior
                // pending tuples must match the type of A.  Since the types
                // do not match, prior updates must be assembled first.
                wait = true ;
            }
            else if
            (
                // the types match, now check the pending operator
                ! (
                    // the operators are the same
                    (accum == C->Pending->op)
                    // or both operators are SECOND_Ctype, implicit or explicit
                    || (GB_op_is_second (accum, C->type) &&
                        GB_op_is_second (C->Pending->op, C->type))
                  )
            )
            { 
                wait = true ;
            }
        }
    }

    if (wait)
    { 
        // Prior computations are not compatible with this assignment, so all
        // prior work must be finished.  This potentially costly.
        // delete any lingering zombies and assemble any pending tuples
        ASSERT_MATRIX_OK (C, "C before wait", GB0) ;
        GB_MATRIX_WAIT (C) ;
    }

    ASSERT_MATRIX_OK (C, "C before subassign", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for assign", GB0) ;

    //--------------------------------------------------------------------------
    // check again if C is empty
    //--------------------------------------------------------------------------

    // GB_clear or GB_Matrix_wait, above, may have deleted all the zombies in
    // C, so check again if C is empty.

    C_is_empty = (GB_NNZ (C) == 0 && !GB_PENDING (C) && !GB_ZOMBIES (C)) ;
    if (C_is_empty)
    { 
        // C is completely empty.  C_replace is irrelevant so set it to false.
        GBURBLE ("(C empty) ") ;
        (*C_replace) = false ;
    }

    //--------------------------------------------------------------------------
    // keep track of the current accum operator
    //--------------------------------------------------------------------------

    // If accum is NULL and pending tuples are added, they will be assembled
    // sometime later (not here) using the implied SECOND_Ctype operator.  This
    // GB_subassigner operation corresponds to C(I,J)=A or C(I,J)<M>=A.
    // Subsequent calls to GrB_setElement, and subsequent calls to GrB_assign
    // or GxB_subassign with an explict SECOND_Ctype operator, may create
    // additional pending tuples and add them to the list without requiring
    // that they be assembled first.

    // If accum is non-NULL, then all prior pending tuples have the same
    // pending operator as this accum.  If that prior operator was the implicit
    // SECOND_Ctype and those pending tuples still exist, then this accum
    // operator is the explicit SECOND_ctype operator.  The implicit
    // SECOND_Ctype operator is replaced with the current accum, which is the
    // explicit SECOND_Ctype operator.

    if (C->Pending != NULL)
    { 
        C->Pending->op = accum ;
    }

    //--------------------------------------------------------------------------
    // return results
    //--------------------------------------------------------------------------

    (*Chandle) = C ;            // C is C_in or C2
    (*Mhandle) = M ;            // M is M_in or M2
    (*Ahandle) = A ;            // A is A_in or A2

    (*C2_handle) = C2 ;
    (*M2_handle) = (MT != NULL) ? MT : M2 ;
    (*A2_handle) = (AT != NULL) ? AT : A2 ;

    (*atype_handle) = atype ;

    // modified versions of the Rows/Cols lists, and their analysis:
    (*I_handle) = I ;           // either Rows, Cols, or I2
    (*I2_handle) = I2 ;         // temporary sorted copy of Rows or Cols list
    (*ni_handle) = ni ;
    (*nI_handle) = nI ;
    (*Ikind_handle) = Ikind ;

    (*J_handle) = J ;           // either Rows, Cols, or J2
    (*J2_handle) = J2 ;         // temporary sorted copy of Rows or Cols list
    (*nj_handle) = nj ;
    (*nJ_handle) = nJ ;
    (*Jkind_handle) = Jkind ;

    return (GrB_SUCCESS) ;
}


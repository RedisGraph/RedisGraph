//------------------------------------------------------------------------------
// GB_assign: submatrix assignment: C<M>(Rows,Cols) = accum (C(Rows,Cols),A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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
#include "GB_assign_zombie.h"
#include "GB_subassign.h"
#include "GB_subref.h"
#include "GB_bitmap_assign.h"

#define GB_FREE_ALL             \
{                               \
    GB_Matrix_free (&C2) ;      \
    GB_Matrix_free (&M2) ;      \
    GB_Matrix_free (&A2) ;      \
    GB_FREE (I2) ;              \
    GB_FREE (J2) ;              \
    GB_Matrix_free (&SubMask) ; \
}

GrB_Info GB_assign                  // C<M>(Rows,Cols) += A or A'
(
    GrB_Matrix C_in,                // input/output matrix for results
    bool C_replace,                 // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C
    const bool Mask_comp,           // true if mask is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    const bool M_transpose,         // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    const bool A_transpose,         // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    int assign_kind,                // row assign, col assign, or assign
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check and prep inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix C = NULL ;           // C_in or C2
    GrB_Matrix M = NULL ;           // M_in or M2
    GrB_Matrix A = NULL ;           // A_in or A2
    GrB_Index *I = NULL ;           // Rows, Cols, or I2
    GrB_Index *J = NULL ;           // Rows, Cols, or J2

    // temporary matrices and arrays
    GrB_Matrix C2 = NULL ;
    GrB_Matrix M2 = NULL ;
    GrB_Matrix A2 = NULL ;
    GrB_Index *I2  = NULL ;
    GrB_Index *J2  = NULL ;
    GrB_Matrix SubMask = NULL ;

    GrB_Type atype = NULL ;
    bool done = false ;
    int64_t ni, nj, nI, nJ, Icolon [3], Jcolon [3] ;
    int Ikind, Jkind ;
    ASSERT_MATRIX_OK (C_in, "C_in for assign", GB0) ;

    GB_OK (GB_assign_prep (&C, &M, &A, &C2, &M2, &A2,
        &I, &I2, &ni, &nI, &Ikind, Icolon,
        &J, &J2, &nj, &nJ, &Jkind, Jcolon,
        &done, &atype, C_in, &C_replace, &assign_kind,
        M_in, Mask_comp, Mask_struct, M_transpose, accum,
        A_in, A_transpose, Rows, nRows_in, Cols, nCols_in,
        scalar_expansion, scalar, scalar_code, Context)) ;

    ASSERT_MATRIX_OK (C, "initial C for assign", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "initial M for assign", GB0) ;

    if (done)
    { 
        // GB_assign_prep has handled the entire assignment itself
        ASSERT_MATRIX_OK (C_in, "QUICK : Final C for assign", GB0) ;
        ASSERT (C == C_in) ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // determine method for GB_subassigner
    //--------------------------------------------------------------------------

    int subassign_method = GB_subassigner_method (C, C_replace,
        M, Mask_comp, Mask_struct, accum, A, Ikind, Jkind, scalar_expansion) ;

    //--------------------------------------------------------------------------
    // determine if the final C_replace phase is needed
    //--------------------------------------------------------------------------

    // whole_submatrix is true if C(:,:)=A is being computed (the submatrix is
    // all of C), or all that the operation can modify for row/col assign.

    bool whole_submatrix ;
    bool whole_C_matrix = (Ikind == GB_ALL && Jkind == GB_ALL) ;
    if (assign_kind == GB_ROW_ASSIGN)
    { 
        // C(i,:) = ... row assignment to the entire row
        whole_submatrix = (Jkind == GB_ALL) ;
    }
    else if (assign_kind == GB_COL_ASSIGN)
    { 
        // C(:,j) = ... col assignment to the entire column
        whole_submatrix = (Ikind == GB_ALL) ;
    }
    else
    { 
        // C(:,:) = ... matrix assignment to the entire matrix
        whole_submatrix = whole_C_matrix ;
    }

    // Mask_is_same is true if SubMask == M (:,:)
    bool Mask_is_same = (M == NULL || whole_submatrix) ;

    // C_replace_phase is true if a final pass over all of C is required
    // to delete entries outside the C(I,J) submatrix.
    bool C_replace_phase = (C_replace && !Mask_is_same) ;

    if ((GB_IS_BITMAP (C) || GB_IS_FULL (C)) && C_replace_phase)
    { 
        // GB_subassigner_method might not select the bitmap assignment
        subassign_method = GB_SUBASSIGN_METHOD_BITMAP ;
    }

    //--------------------------------------------------------------------------
    // do the assignment
    //--------------------------------------------------------------------------

    if (subassign_method == GB_SUBASSIGN_METHOD_BITMAP)
    { 

        //----------------------------------------------------------------------
        // use GB_bitmap_assign directly
        //----------------------------------------------------------------------

        // GB_bitmap_assign does not need to create the SubMask, and it also
        // handles the C_replace_phase itself.  C is bitmap, or is converted to
        // bitmap by GB_bitmap_assign, before the assignment.  For the C = A
        // and C = scalar assignment, C may be returned in any sparsity
        // structure, but otherwise C is returned as bitmap.

        GB_OK (GB_bitmap_assign (C, C_replace,
            I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
            M, Mask_comp, Mask_struct, accum, A,
            scalar, atype, assign_kind, Context)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // use GB_subassigner
        //----------------------------------------------------------------------

        // M and A can have any sparsity structure.  C is typically not
        // bitmap, except

        //----------------------------------------------------------------------
        // extract the SubMask = M (I,J) if needed
        //----------------------------------------------------------------------

        if (Mask_is_same)
        { 
            // the mask M is the same for GB_assign and GB_subassign.  Either
            // both masks are NULL, or SubMask = M (:,:), and the two masks
            // are equivalent.

            //------------------------------------------------------------------
            // C(I,J)<M> = A or accum (C(I,J),A) via GB_subassigner
            //------------------------------------------------------------------

            GB_OK (GB_subassigner (C, subassign_method, C_replace,
                M, Mask_comp, Mask_struct, accum, A,
                I, ni, nI, Ikind, Icolon, J, nj, nJ, Jkind, Jcolon,
                scalar_expansion, scalar, atype, Context)) ;

        }
        else
        {

            //------------------------------------------------------------------
            // extract the SubMask
            //------------------------------------------------------------------

            ASSERT_MATRIX_OK (M, "big mask", GB0) ;

            const GrB_Index *I_SubMask = I ; int64_t ni_SubMask = ni ;
            const GrB_Index *J_SubMask = J ; int64_t nj_SubMask = nj ;

            if (assign_kind == GB_ROW_ASSIGN)
            { 
                // SubMask = M (:,J)
                ASSERT (M->vlen == 1 && M->vdim == C->vdim) ;
                I_SubMask = GrB_ALL ;
                ni_SubMask = 1 ;
            }
            else if (assign_kind == GB_COL_ASSIGN)
            { 
                // SubMask = M (I,:)
                ASSERT (M->vlen == C->vlen && M->vdim == 1) ;
                J_SubMask = GrB_ALL ;
                nj_SubMask = 1 ;
            }
            else // assign_kind == GB_ASSIGN
            { 
                // SubMask = M (I,J)
                ASSERT (M->vlen == C->vlen && M->vdim == C->vdim) ;
            }

            GB_OK (GB_subref (&SubMask, true, M,
                I_SubMask, ni_SubMask, J_SubMask, nj_SubMask,
                false, Context)) ;

            // GB_subref can return a jumbled result
            ASSERT (GB_JUMBLED_OK (SubMask)) ;
            ASSERT_MATRIX_OK (SubMask, "extracted SubMask", GB0) ;

            //------------------------------------------------------------------
            // C(I,J)<SubMask> = A or accum (C(I,J),A) via GB_subassigner
            //------------------------------------------------------------------

            // determine the method again since SubMask is not M
            subassign_method = GB_subassigner_method (C, C_replace,
                SubMask, Mask_comp, Mask_struct, accum, A, Ikind, Jkind,
                scalar_expansion) ;

            GB_OK (GB_subassigner (C, subassign_method, C_replace,
                SubMask, Mask_comp, Mask_struct, accum, A,
                I, ni, nI, Ikind, Icolon, J, nj, nJ, Jkind, Jcolon,
                scalar_expansion, scalar, atype, Context)) ;

            GB_Matrix_free (&SubMask) ;
        }

        //----------------------------------------------------------------------
        // examine C outside the C(I,J) submatrix
        //----------------------------------------------------------------------

        if (C_replace_phase)
        {
            // If C_replace is true and M(i,j)=0 for any entry outside the
            // C(I,J) submatrix, then that entry must be deleted.  This phase
            // is very costly but it is what the GraphBLAS Specification
            // requires.  This phase is skipped if C_replace is false.

            // This case can only occur if the mask is present (either
            // complemented or not).  If the mask is not present, then it is
            // not complemented (see the "quick return" case above).  So if
            // there is no mask matrix, M(I,J)=1 is true, so C_replace has no
            // effect outside the C(I,J) submatrix.

            // Also, if whole_submatrix is true, then there is nothing outside
            // the C(I,J) submatrix to modify, so this phase is skipped if
            // whole_submatrix is true.

            // This code requires C and M not to be aliased to each other.
            ASSERT (M != NULL) ;
            ASSERT (!GB_aliased (C, M)) ;   // NO ALIAS C==M in C_replace_phase
            ASSERT (!whole_submatrix) ;
            ASSERT (!GB_IS_BITMAP (C)) ;
            ASSERT (!GB_IS_FULL (C)) ;

            ASSERT_MATRIX_OK (C, "C for C-replace-phase", GB0) ;
            ASSERT_MATRIX_OK (M, "M for C-replace-phase", GB0) ;

            //------------------------------------------------------------------
            // assemble any pending tuples
            //------------------------------------------------------------------

            GB_MATRIX_WAIT_IF_PENDING (C) ;
            ASSERT_MATRIX_OK (C, "C cleaned up for C-replace-phase", GB0) ;

            //------------------------------------------------------------------
            // delete entries outside C(I,J) for which M(i,j) is false
            //------------------------------------------------------------------

            // C must be sparse or hypersparse
            GB_ENSURE_SPARSE (C) ;

            if (assign_kind == GB_COL_ASSIGN)
            { 

                //--------------------------------------------------------------
                // vector assignment, examine all of M but just C(:,j)
                //--------------------------------------------------------------

                // M is a single column so it is never hypersparse
                ASSERT (nJ == 1) ;
                ASSERT (M->vlen == C->vlen && M->vdim == 1 && M->h == NULL) ;
                int64_t j = GB_ijlist (J, 0, Jkind, Jcolon) ;
                GBURBLE ("assign zombies outside C(I,j) ") ;
                GB_MATRIX_WAIT (M) ;
                GB_assign_zombie3 (C, M, Mask_comp, Mask_struct,
                    j, I, nI, Ikind, Icolon, Context) ;

            }
            else if (assign_kind == GB_ROW_ASSIGN)
            { 

                //--------------------------------------------------------------
                // index assignment, examine just C(i,:) and M
                //--------------------------------------------------------------

                // GrB_Row_assign: only examine C(i,:)
                // M s a single row with vlen == 1 and the same vdim as C
                ASSERT (nI == 1) ;
                ASSERT (M->vlen == 1 && M->vdim == C->vdim) ;
                int64_t i = GB_ijlist (I, 0, Ikind, Icolon) ;
                GBURBLE ("assign zombies outside C(i,J) ") ;
                GB_MATRIX_WAIT_IF_JUMBLED (C) ;
                GB_MATRIX_WAIT (M) ;
                GB_assign_zombie4 (C, M, Mask_comp, Mask_struct,
                    i, J, nJ, Jkind, Jcolon, Context) ;

            }
            else
            { 

                //--------------------------------------------------------------
                // Matrix/vector assignment: examine all of C and M
                //--------------------------------------------------------------

                // M has the same size as C
                ASSERT (M->vlen == C->vlen && M->vdim == C->vdim) ;
                GBURBLE ("assign zombies outside C(I,J) ") ;
                GB_MATRIX_WAIT (M) ;
                GB_OK (GB_assign_zombie5 (C, M, Mask_comp, Mask_struct,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon, Context)) ;
            }
            ASSERT_MATRIX_OK (C, "C for C-replace-phase done", GB_FLIP (GB0)) ;
        }
    }

    //--------------------------------------------------------------------------
    // transplant C2 back into C_in
    //--------------------------------------------------------------------------

    if (C == C2)
    { 
        // Transplant the content of C2 into C_in and free C2.  Zombies and
        // pending tuples can be transplanted from C2 into C_in, and if C2 is
        // jumbled, C_in becomes jumbled too.
        GB_OK (GB_transplant (C_in, C_in->type, &C2, Context)) ;
    }

    //--------------------------------------------------------------------------
    // free workspace, finalize C, and return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C_in, "C to conform", GB0) ;
    GB_OK (GB_conform (C_in, Context)) ;
    ASSERT_MATRIX_OK (C_in, "Final C for assign", GB0) ;
    GB_FREE_ALL ;
    return (GB_block (C_in, Context)) ;
}


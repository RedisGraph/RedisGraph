//------------------------------------------------------------------------------
// GB_subassign: C(Rows,Cols)<M> = accum (C(Rows,Cols),A) or A'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// submatrix assignment: C(Rows,Cols)<M> = accum (C(Rows,Cols),A)

// All GxB_*_subassign operations rely on this function.

// With scalar_expansion = false, this method does the work for the standard
// GxB_*_subassign operations (GxB_Matrix_subassign, GxB_Vector_subassign,
// GxB_Row_subassign, and GxB_Col_subassign).  If scalar_expansion is true, it
// performs scalar assignment (the GxB_*_subassign_TYPE functions) in which
// case the input matrix A is ignored (it is NULL), and the scalar is used
// instead.

// Compare with GB_assign, which uses M and C_replace differently

#include "GB_subassign.h"
#include "GB_transpose.h"

#define GB_FREE_ALL                                 \
{                                                   \
    GB_MATRIX_FREE (&Z2) ;                          \
    GB_MATRIX_FREE (&AT) ;                          \
    GB_MATRIX_FREE (&MT) ;                          \
}

GrB_Info GB_subassign               // C(Rows,Cols)<M> += A or A'
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C(Rows,Cols)
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

    // C may be aliased with M_in and/or A_in

    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL (Rows) ;
    GB_RETURN_IF_NULL (Cols) ;

    GrB_Info info ;
    GrB_Matrix M = M_in ;
    GrB_Matrix A = A_in ;

    if (scalar_expansion)
    { 
        // for scalar expansion, the NULL pointer case has been already checked
        // for user-defined types, and can't be NULL for built-in types.
        ASSERT (scalar != NULL) ;
        ASSERT (A == NULL) ;
    }
    else
    { 
        // GrB_*assign, not scalar:  The user's input matrix has been checked.
        // The pointer to the scalar is NULL.
        ASSERT (scalar == NULL) ;
        ASSERT_MATRIX_OK (A, "A for GB_subassign", GB0) ;
    }

    ASSERT_MATRIX_OK (C, "C input for GB_subassign", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_subassign", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_subassign", GB0) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    int64_t nRows, nCols, RowColon [3], ColColon [3] ;
    int RowsKind, ColsKind ;
    GB_ijlength (Rows, nRows_in, GB_NROWS (C), &nRows, &RowsKind, RowColon) ;
    GB_ijlength (Cols, nCols_in, GB_NCOLS (C), &nCols, &ColsKind, ColColon) ;

    bool whole_C_matrix = (RowsKind == GB_ALL && ColsKind == GB_ALL) ;

    GrB_Matrix AT = NULL ;
    GrB_Matrix MT = NULL ;
    GrB_Matrix Z2 = NULL ;
    GrB_Matrix Z = NULL ;

    bool C_is_csc = C->is_csc ;

    //--------------------------------------------------------------------------
    // check domains and dimensions for C(Rows,Cols)<M> += A or A'
    //--------------------------------------------------------------------------

    // GB_compatible is not used since most of it is slightly different here
    if (accum != NULL)
    { 
        // C(Rows,Cols)<M> = accum (C(Rows,Cols),A)
        GB_OK (GB_BinaryOp_compatible (accum, C->type, C->type,
            (scalar_expansion) ? NULL : A->type,
            (scalar_expansion) ? scalar_code : GB_ignore_code, Context)) ;
    }

    // C(Rows,Cols)<M> = T, so C and T must be compatible.
    // also C(Rows,Cols)<M> = accum(C,T) for entries in T but not C
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
        // M is a matrix the same size as C(Rows,Cols)
        int64_t mnrows = M_transpose ? GB_NCOLS (M) : GB_NROWS (M) ;
        int64_t mncols = M_transpose ? GB_NROWS (M) : GB_NCOLS (M) ;
        if (mnrows != nRows || mncols != nCols)
        { 
            return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                "M is "GBd"-by-"GBd"%s, "
                "must match size of result C(I,J): "GBd"-by-"GBd"",
                mnrows, mncols, M_transpose ? " (transposed)" : "",
                nRows, nCols))) ;
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

    const GrB_Index *I, *J ;
    int64_t ni, nj ;

    if (!scalar_expansion && C_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    if (C_is_csc)
    { 
        // C is in CSC format
        I = Rows ; ni = nRows_in ;     // indices into the vectors
        J = Cols ; nj = nCols_in ;     // vectors
    }
    else
    { 
        // C is in CSR format
        I = Cols ; ni = nCols_in ;     // indices into the vectors
        J = Rows ; nj = nRows_in ;     // vectors
    }

    // C has C->vdim vectors, each of length C->vlen.
    // J is a list of length |J| of vectors in the range 0:C->vdim-1.
    // I is a list of length |I| of indices in the range 0:C->vlen-1.

    //--------------------------------------------------------------------------
    // transpose A if requested
    //--------------------------------------------------------------------------

    if (!scalar_expansion && A_transpose)
    { 
        // AT = A', with no typecasting
        // transpose: no typecast, no op, not in place
        GBBURBLE ("(A transpose) ") ;
        GB_OK (GB_transpose (&AT, NULL, C_is_csc, A, NULL, Context)) ;
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
            // MT = M' to conform M to the same CSR/CSC format as C.
            // typecast to boolean, if a full matrix transpose is done.
            // transpose: no typecast, no op, not in place
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
            // is need.  Instead of duplicating it, create an empty matrix Z2.
            // This also prevents the C_replace_phase from being needed.
            GB_NEW (&Z2, C->type, C->vlen, C->vdim, GB_Ap_calloc,
                C->is_csc, GB_SAME_HYPER_AS (C->is_hyper), C->hyper_ratio,
                1, Context) ;
            GB_OK (info)  ;
            GBBURBLE ("(C alias cleared; C_replace early) ") ;
            C_replace = false ;
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
        // GB_subassigner can safely operate on C in place.
        // FUTURE:  if C is dense and will remain so,
        // it would be faster to delay the clearing of C.
        if (whole_C_matrix && C_replace && accum == NULL)
        { 
            // C(:,:)<any mask, replace> = A or x, with C not aliased to M or
            // A.  C is about to be cleared in GB_subassigner anyway, so clear
            // it now.
            GB_OK (GB_clear (C, Context)) ;
            GBBURBLE ("(C(:,:)<any mask>: C_replace early) ") ;
            C_replace = false ;
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

    // Z2 is still needed
    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&MT) ;

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
        // transplants the content of Z2 into C and frees Z2
        GB_OK (GB_transplant (C, C->type, &Z2, Context)) ;
    }

    // The hypersparsity of C is not modified.  This will be done eventually,
    // when all pending operations are completed via GB_wait.

    //--------------------------------------------------------------------------
    // free workspace, finalize C, and return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "Final C for subassign", GB0) ;
    GB_FREE_ALL ;
    return (GB_block (C, Context)) ;
}


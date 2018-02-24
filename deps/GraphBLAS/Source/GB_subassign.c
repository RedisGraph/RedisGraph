//------------------------------------------------------------------------------
// GB_subassign: submatrix assignment: C(I,J)<Mask> = accum (C(I,J),A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// submatrix assignment: C(I,J)<Mask> = accum (C(I,J),A)
// compare/contrast this function with GB_assign.

// All GxB_*_subassign operations rely on this function.

// With scalar_expansion = false, this method does the work for the standard
// GxB_*subassign operations (GxB_Matrix_subassign, GxB_Vector_subassign,
// GxB_Row_subassign, and GxB_Col_subassign).  If scalar_expansion is true, it
// performs scalar assignment (the GxB_*_subassign_TYPE functions) in which
// case the input matrix A is ignored (it is NULL), and the scalar is used
// instead.

#include "GB.h"

GrB_Info GB_subassign               // C(I,J)<Mask> = accum (C(I,J),A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code  // type code of scalar to expand
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C, Mask, and A checked in the caller
    RETURN_IF_UNINITIALIZED (accum) ;
    RETURN_IF_NULL (I) ;            // I = GrB_ALL is not NULL
    RETURN_IF_NULL (J) ;            // J = GrB_ALL is not NULL

    if (scalar_expansion)
    {
        // GB_subassign_scalar: for scalar expansion, the NULL pointer case has
        // been already checked for user-defined types, and can't be NULL for
        // built-in types.
        ASSERT (scalar != NULL) ;
        ASSERT (A == NULL) ;
    }
    else
    {
        // GrB_*assign, not scalar:  The user's input matrix has been checked.
        // The pointer to the scalar is NULL.
        ASSERT (scalar == NULL) ;
        ASSERT_OK (GB_check (A, "A for GB_subassign", 0)) ;
    }

    ASSERT_OK (GB_check (C, "C input for GB_subassign", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_subassign", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_subassign", 0)) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

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
    // check domains and dimensions for C(I,J)<Mask> = accum (C(I,J),A)
    //--------------------------------------------------------------------------

    // GB_compatible is not used since most of it is slightly different here
    if (accum != NULL)
    {
        // C(I,J)<Mask> = accum (C(I,J),A)
        info = GB_BinaryOp_compatible (accum, C->type, C->type,
            (scalar_expansion) ? NULL : A->type,
            (scalar_expansion) ? scalar_code : 0) ;
        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }
    }

    // C(I,J)<Mask> = T, so C and T must be compatible.
    // also C(I,J)<Mask> = accum(C,T) for entries in T but not C
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

    // check the mask: must be the same size as C(I,J)
    info = GB_Mask_compatible (Mask, NULL, ni, nj) ;
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
    // transpose A if requested
    //--------------------------------------------------------------------------

    GrB_Matrix A2 ;
    GrB_Matrix AT = NULL ;

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
            return (info) ;
        }
        info = GB_Matrix_transpose (AT, A, NULL, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
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
    // Z = C
    //--------------------------------------------------------------------------

    // GB_subassign_kernel modifies C efficiently in place, but it can only do
    // so if C is not aliased with A2 or Mask.  If C is aliased a copy must
    // be made.  GB_subassign_kernel operates on the copy, Z, which is then
    // transplanted back into C when done.  This is costly, and can have
    // performance implications, but it is the only reasonable method.  If C is
    // aliased, then the assignment is a large one and copying the whole matrix
    // will not add much time.

    GrB_Matrix Z ;
    bool aliased = (C == A2 || C == Mask) ;
    if (aliased)
    {
        // Z = duplicate of C
        info = GB_Matrix_dup (&Z, C) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            return (info) ;
        }
    }
    else
    {
        // GB_subassign_kernel can safely operate on C in place
        Z = C ;
    }

    //--------------------------------------------------------------------------
    // Z(I,J)<Mask> = A or accum (Z(I,J),A), no transpose of A
    //--------------------------------------------------------------------------

    info = GB_subassign_kernel (
        Z,          C_replace,      // Z matrix and its descriptor
        Mask,       Mask_comp,      // Mask matrix and its descriptor
        accum,                      // for accum (C(I,J),A)
        A2,                         // A matrix, NULL for scalar expansion
        I, ni,                      // row indices
        J, nj,                      // column indices
        scalar_expansion,           // if true, expand scalar to A
        scalar,                     // scalar to expand, NULL if A not NULL
        scalar_code) ;              // type code of scalar to expand

    GB_MATRIX_FREE (&AT) ;

    //--------------------------------------------------------------------------
    // C = Z
    //--------------------------------------------------------------------------

    if (aliased)
    {
        if (info == GrB_SUCCESS)
        {
            // zombies can be transplanted into C but pending tuples cannot
            if (Z->npending > 0)
            {
                // assemble all pending tuples, and delete all zombies too
                info = GB_wait (Z) ;
            }
        }
        if (info == GrB_SUCCESS)
        {
            // transplants the content of Z into C and frees Z
            return (GB_Matrix_transplant (C, C->type, &Z)) ;
        }
        else
        {
            // Z needs to be freed if C is aliased but info != GrB_SUCCESS.
            // (out of memory, or inputs invalid).  C remains unchanged.
            GB_MATRIX_FREE (&Z) ;
        }
    }

    return (info) ;             // pass info directly from GB_subassign_kernel
}


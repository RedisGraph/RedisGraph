//------------------------------------------------------------------------------
// GrB_transpose: transpose a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C,A') or accum(C,A)

#include "GB.h"

GrB_Info GrB_transpose              // C<Mask> = accum(C,A') or accum(C,A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_transpose (C, Mask, accum, A, desc)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_UNINITIALIZED (Mask) ;
    RETURN_IF_UNINITIALIZED (accum) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    ASSERT_OK (GB_check (C, "C input for GrB_transpose", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GrB_transpose", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GrB_transpose", 0)) ;
    ASSERT_OK (GB_check (A, "A input for GrB_transpose", 0)) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, ignore) ;

    // check domains and dimensions for C<Mask> = accum (C,T)
    info = GB_compatible (C->type, C, Mask, accum, A->type) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // check the dimensions
    int64_t tnrows = (!A_transpose) ? A->ncols : A->nrows ;
    int64_t tncols = (!A_transpose) ? A->nrows : A->ncols ;
    if (C->nrows != tnrows || C->ncols != tncols)
    {
        return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "input is "GBd"-by-"GBd"%s",
            C->nrows, C->ncols,
            tnrows, tncols, (!A_transpose) ? " (transposed)" : ""))) ;
    }

    // quick return if an empty Mask is complemented
    RETURN_IF_QUICK_MASK (C, C_replace, Mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (C) ;
    APPLY_PENDING_UPDATES (Mask) ;
    APPLY_PENDING_UPDATES (A) ;

    //--------------------------------------------------------------------------
    // T = A or A', where T can have the type of C or the type of A
    //--------------------------------------------------------------------------

    GrB_Matrix T = NULL ;
    GrB_Type T_type ;

    if (!A_transpose)
    {
        // T = A', the default behavior.  This step may seem counter-intuitive,
        // but method computes C<Mask>=A' by default when A_transpose is false.

        // Precasting:
        if (accum == NULL)
        {
            // If there is no accum operator, T is transplanted into Z and
            // typecasted into the C->type.  This can be done during the
            // transpose below.
            T_type = C->type ;
        }
        else
        {
            // If the accum operator is present, entries in the intersection of
            // T and C are typecasted into the accum->ytype, while entries in T
            // but not C are typecasted directly into C->type.  Thus, the
            // typecast of T (if any) must wait, and be done in call to
            // GB_Matrix_add in GB_accum_mask.
            T_type = A->type ;
        }

        // [ T->p malloc'ed, not initialized
        GB_NEW (&T, T_type, C->nrows, C->ncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }

        // no operator; typecasting done if accum is NULL
        info = GB_Matrix_transpose (T, A, NULL, true) ;
        // T->p now initialzed ]
    }
    else
    {
        // T = A, a pure shallow copy; nothing at all is allocated.  No
        // typecasting is done since the types of T and A are the same.  If the
        // A_transpose descriptor is true, A is viewed as transposed first.
        // The method transposes A again, giving T=A''=A.  This is a double
        // transpose, so C<Mask>=A is computed, and no transpose is done.
        // T is typecasted eventually, into the type of C if the types of
        // T and C differ.  That can be postponed at no cost since the
        // following step is free.
        T_type = A->type ;
        info = GB_shallow_cast (&T, T_type, A) ;
    }

    if (info != GrB_SUCCESS)
    {
        GB_MATRIX_FREE (&T) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
}


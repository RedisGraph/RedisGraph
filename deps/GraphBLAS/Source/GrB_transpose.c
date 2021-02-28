//------------------------------------------------------------------------------
// GrB_transpose: transpose a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,A') or accum (C,A)

#include "GB_transpose.h"
#include "GB_accum_mask.h"

GrB_Info GrB_transpose              // C<M> = accum(C,A') or accum(C,A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, M, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M and/or A

    GB_WHERE ("GrB_transpose (C, M, accum, A, desc)") ;
    GB_BURBLE_START ("GrB_transpose") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    ASSERT_MATRIX_OK (C, "C input for GrB_transpose", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GrB_transpose", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GrB_transpose", GB0) ;
    ASSERT_MATRIX_OK (A, "A input for GrB_transpose", GB0) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_transpose, xx1, xx2) ;

    // check domains and dimensions for C<M> = accum (C,T)
    info = GB_compatible (C->type, C, M, accum, A->type, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // check the dimensions
    int64_t tnrows = (!A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t tncols = (!A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    if (GB_NROWS (C) != tnrows || GB_NCOLS (C) != tncols)
    { 
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "input is "GBd"-by-"GBd"%s",
            GB_NROWS (C), GB_NCOLS (C),
            tnrows, tncols, (!A_transpose) ? " (transposed)" : ""))) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    // GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;

    //--------------------------------------------------------------------------
    // T = A or A', where T can have the type of C or the type of A
    //--------------------------------------------------------------------------

    bool C_is_csc = C->is_csc ;
    if (C_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    GrB_Matrix T = NULL ;

    if (!A_transpose)
    {

        // T = A', the default behavior.  This step may seem counter-intuitive,
        // but method computes C<M>=A' by default when A_transpose is false.
        GBBURBLE ("(transpose) ") ;

        // Precasting:
        if (accum == NULL)
        { 
            // If there is no accum operator, T is transplanted into Z and
            // typecasted into the C->type during the transpose.
            // transpose: typecast, no op, not in place
            info = GB_transpose (&T, C->type, C_is_csc, A, NULL, Context) ;
        }
        else
        { 
            // If the accum operator is present, entries in the intersection of
            // T and C are typecasted into the accum->ytype, while entries in T
            // but not C are typecasted directly into C->type.  Thus, the
            // typecast of T (if any) must wait, and be done in call to GB_add
            // in GB_accum_mask.
            // transpose: no typecast, no op, not in place
            info = GB_transpose (&T, A->type, C_is_csc, A, NULL, Context) ;
        }

        // no operator; typecasting done if accum is NULL
    }
    else
    { 

        // T = A, a pure shallow copy; nothing at all is allocated.  No
        // typecasting is done since the types of T and A are the same.  If the
        // A_transpose descriptor is true, A is viewed as transposed first.
        // The method transposes A again, giving T=A''=A.  This is a double
        // transpose, so C<M>=A is computed, and no transpose is done.  T is
        // typecasted eventually, into the type of C if the types of T and C
        // differ.  That can be postponed at no cost since the following step
        // is free.
        GBBURBLE ("(cheap) ") ;
        info = GB_shallow_copy (&T, C_is_csc, A, Context) ;
    }

    if (info != GrB_SUCCESS)
    { 
        ASSERT (T == NULL) ;
        return (info) ;
    }

    ASSERT (T->is_csc == C->is_csc) ;
    ASSERT_MATRIX_OK (T, "T for GrB_transpose", GB0) ;
    ASSERT_MATRIX_OK (C, "C for GrB_transpose", GB0) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask M
    //--------------------------------------------------------------------------

    info = GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp, 
        Mask_struct, Context) ;
    ASSERT (T == NULL) ;

    GB_BURBLE_END ;
    return (info) ;
}


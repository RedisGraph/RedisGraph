//------------------------------------------------------------------------------
// GB_mxm: matrix-matrix multiply for GrB_mxm, GrB_mxv, and GrB_vxm
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,A*B) and variations.

// This function is not user-callable.  It does the work for user-callable
// functions GrB_mxm, GrB_mxv, and GrB_vxm.

#include "GB_mxm.h"
#include "GB_accum_mask.h"

GrB_Info GB_mxm                     // C<M> = A*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for C=A*B
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose,         // if true, use B' instead of B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M, A, and/or B

    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;

    ASSERT_MATRIX_OK (C, "C input for GB_mxm", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_mxm", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_mxm", GB0) ;
    ASSERT_SEMIRING_OK (semiring, "semiring for GB_mxm", GB0) ;
    ASSERT_MATRIX_OK (A, "A for GB_mxm", GB0) ;
    ASSERT_MATRIX_OK (B, "B for GB_mxm", GB0) ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Type T_type = semiring->add->op->ztype ;
    GrB_Info info = GB_compatible (C->type, C, M, accum, T_type, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // T=A*B via semiring: A and B must be compatible with semiring->multiply
    if (flipxy)
    { 
        // z=fmult(b,a), for entries a from A, and b from B
        info = GB_BinaryOp_compatible (semiring->multiply,
                NULL, B->type, A->type, GB_ignore_code, Context) ;
    }
    else
    { 
        // z=fmult(a,b), for entries a from A, and b from B
        info = GB_BinaryOp_compatible (semiring->multiply,
                NULL, A->type, B->type, GB_ignore_code, Context) ;
    }
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // check the dimensions
    int64_t anrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t ancols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    int64_t bnrows = (B_transpose) ? GB_NCOLS (B) : GB_NROWS (B) ;
    int64_t bncols = (B_transpose) ? GB_NROWS (B) : GB_NCOLS (B) ;
    if (ancols != bnrows || GB_NROWS (C) != anrows || GB_NCOLS (C) != bncols)
    { 
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "first input is "GBd"-by-"GBd"%s\n"
            "second input is "GBd"-by-"GBd"%s",
            GB_NROWS (C), GB_NCOLS (C),
            anrows, ancols, A_transpose ? " (transposed)" : "",
            bnrows, bncols, B_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    // GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;
    GB_WAIT (B) ;

    //--------------------------------------------------------------------------
    // T = A*B, A'*B, A*B', or A'*B', also using the mask to cut time and memory
    //--------------------------------------------------------------------------

    // If C is dense (with no pending work), and the accum is present, then
    // C+=A*B can be done in place (C_replace is effectively false).  If C is
    // dense, M is present, and C_replace is false, then C<M>+=A*B or
    // C<!M>+=A*B can also be done in place.  In all of these cases, C remains
    // dense.

    bool mask_applied = false ;
    bool done_in_place = false ;
    GrB_Matrix T = NULL, MT = NULL ;
    info = GB_AxB_meta (&T, C, C_replace, C->is_csc, &MT, M, Mask_comp,
        Mask_struct, accum, A, B, semiring, A_transpose, B_transpose, flipxy,
        &mask_applied, &done_in_place, AxB_method, &(C->AxB_method_used),
        Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        ASSERT (T == NULL) ;
        ASSERT (MT == NULL) ;
        return (info) ;
    }

    if (done_in_place)
    { 
        // C<...>+=A*B has been computed in place; no more work to do
        GB_MATRIX_FREE (&MT) ;
        ASSERT_MATRIX_OK (C, "C from GB_mxm (in place)", GB0) ;
        return (info) ;
    }

    ASSERT_MATRIX_OK (T, "T=A*B from GB_AxB_meta", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (MT, "MT from GB_AxB_meta", GB0) ;
    ASSERT (GB_ZOMBIES_OK (T)) ;
    ASSERT (!GB_PENDING (T)) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask
    //--------------------------------------------------------------------------

    if ((accum == NULL) && (C->is_csc == T->is_csc)
        && (M == NULL || (M != NULL && mask_applied))
        && (C_replace || GB_NNZ_UPPER_BOUND (C) == 0))
    { 
        // C = 0 ; C = (ctype) T ; with the same CSR/CSC format.  The mask M
        // (if any) has already been applied.  If C is also empty, or to be
        // cleared anyway, and if accum is not present, then T can be
        // transplanted directly into C, as C = (ctype) T, typecasting if
        // needed.  If no typecasting is done then this takes no time at all
        // and is a pure transplant.  Also conform C to its desired
        // hypersparsity.
        GB_MATRIX_FREE (&MT) ;
        if (GB_ZOMBIES (T) && T->type != C->type)
        { 
            // T = A*B can be constructed with zombies, using the dot3 method.
            // Since its type differs from C, its values will be typecasted
            // from T->type to C->type.  The zombies are killed before
            // typecasting.  Otherwise, if they were not killed, uninitialized
            // values in T->x for these zombies will get typecasted into C->x.
            // Typecasting a zombie is safe, since the values of all zombies
            // are ignored.  But valgrind complains about it, so they are
            // killed now.  Also see the discussion in GB_transplant.
            GBBURBLE ("(wait, so zombies are not typecasted) ") ;
            info = GB_wait (T, Context) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                GB_MATRIX_FREE (&T) ;
                return (info) ;
            }
        }
        info = GB_transplant_conform (C, C->type, &T, Context) ;
        #ifdef GB_DEBUG
        if (info == GrB_SUCCESS)
        {
            // C may be returned with zombies, but no pending tuples
            ASSERT_MATRIX_OK (C, "C from GB_mxm (transplanted)", GB0) ;
            ASSERT (GB_ZOMBIES_OK (C)) ;
            ASSERT (!GB_PENDING (C)) ;
        }
        #endif
    }
    else
    { 
        // C<M> = accum (C,T)
        // GB_accum_mask also conforms C to its desired hypersparsity
        info = GB_accum_mask (C, M, MT, accum, &T, C_replace, Mask_comp,
            Mask_struct, Context) ;
        GB_MATRIX_FREE (&MT) ;
        #ifdef GB_DEBUG
        if (info == GrB_SUCCESS)
        {
            // C may be returned with zombies and pending tuples
            ASSERT_MATRIX_OK (C, "Final C from GB_mxm (accum_mask)", GB0) ;
            ASSERT (GB_ZOMBIES_OK (C)) ;
            ASSERT (GB_PENDING_OK (C)) ;
        }
        #endif
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (info) ;
}


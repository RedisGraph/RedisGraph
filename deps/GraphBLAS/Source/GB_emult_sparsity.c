//------------------------------------------------------------------------------
// GB_emult_sparsity: determine the sparsity structure for C<M or !M>=A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Determines the sparsity structure for C, for computing C=A.*B, C<M>=A.*B,
// or C<!M>=A.*B, based on the sparsity structures of M, A, and B, and whether
// or not M is complemented.  It also decides if the mask M should be applied
// by GB_emult, or if C=A.*B should be computed without the mask, and the mask
// applied later.

// If C should be constructed as hypersparse or sparse, this function simply
// returns GxB_SPARSE.  The final determination is made by GB_emult_phase0.

// If both A and B are full, then GB_ewise calls GB_add instead of GB_emult.
// This is the only case where the eWise multiply can produce a full C matrix,
// and as a result, there is no need for a GB_emult to handle the case when
// C is full.

#include "GB_emult.h"

int GB_emult_sparsity       // return the sparsity structure for C
(
    // output:
    bool *apply_mask,       // if true then mask will be applied by GB_emult
    bool *use_add_instead,  // if true then use GB_add instead of GB_emult
    // input:
    const GrB_Matrix M,     // optional mask for C, unused if NULL
    const bool Mask_comp,   // if true, use !M
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B      // input B matrix
)
{

    //--------------------------------------------------------------------------
    // determine the sparsity of C
    //--------------------------------------------------------------------------

    // Unless deciding otherwise, use the mask if it appears
    (*apply_mask) = (M != NULL) ;

    int C_sparsity ;

    // In the table below, sparse/hypersparse are listed as "sparse".  If C is
    // listed as sparse: it is hypersparse if M is hypersparse (and not
    // complemented), or if both A and B are hypersparse, and sparse otherwise.
    // This is determined by GB_emult_phase0.

    bool M_is_sparse_or_hyper = GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M) ;
    bool A_is_sparse_or_hyper = GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A) ;
    bool B_is_sparse_or_hyper = GB_IS_SPARSE (B) || GB_IS_HYPERSPARSE (B) ;
    bool A_is_full = GB_as_if_full (A) ;
    bool B_is_full = GB_as_if_full (B) ;

    // Methods labeled as "use GB_add" give the same results with GB_add and
    // GB_emult, when A and B are both full.  For those cases, GB_ewise should
    // call GB_add instead of GB_add.
    (*use_add_instead) = A_is_full && B_is_full ;

    if (M == NULL)
    {

        //      ------------------------------------------
        //      C       =           A       .*      B
        //      ------------------------------------------
        //      sparse  .           sparse          sparse
        //      sparse  .           sparse          bitmap
        //      sparse  .           sparse          full  
        //      sparse  .           bitmap          sparse
        //      bitmap  .           bitmap          bitmap
        //      bitmap  .           bitmap          full  
        //      sparse  .           full            sparse
        //      bitmap  .           full            bitmap
        //      full    .           full            full    (use GB_add)

        if (A_is_sparse_or_hyper || B_is_sparse_or_hyper)
        { 
            // C=A.*B with A or B sparse/hyper, C sparse
            C_sparsity = GxB_SPARSE ;
        }
        else if (A_is_full && B_is_full)
        { 
            // C=A.*B with A and B full, C full
            C_sparsity = GxB_FULL ;
        }
        else
        { 
            // C=A.*B, otherwise, C bitmap
            C_sparsity = GxB_BITMAP ;
        }

    }
    else if (!Mask_comp)
    {

        if (M_is_sparse_or_hyper)
        { 

            //      ------------------------------------------
            //      C       <M>=        A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      sparse          sparse
            //      sparse  sparse      sparse          bitmap
            //      sparse  sparse      sparse          full  
            //      sparse  sparse      bitmap          sparse
            //      sparse  sparse      bitmap          bitmap
            //      sparse  sparse      bitmap          full  
            //      sparse  sparse      full            sparse
            //      sparse  sparse      full            bitmap
            //      sparse  sparse      full            full    (use GB_add)

            // TODO: check same condition as GB_add, for very sparse mask M.

            // C<M>=A.*B with M sparse/hyper, C sparse
            C_sparsity = GxB_SPARSE ;

        }
        else
        {

            //      ------------------------------------------
            //      C      <M> =        A       .*      B
            //      ------------------------------------------
            //      sparse  bitmap      sparse          sparse
            //      sparse  bitmap      sparse          bitmap
            //      sparse  bitmap      sparse          full  
            //      sparse  bitmap      bitmap          sparse
            //      bitmap  bitmap      bitmap          bitmap
            //      bitmap  bitmap      bitmap          full  
            //      sparse  bitmap      full            sparse
            //      bitmap  bitmap      full            bitmap
            //      bitmap  bitmap      full            full    (use GB_add)

            //      ------------------------------------------
            //      C      <M> =        A       .*      B
            //      ------------------------------------------
            //      sparse  full        sparse          sparse
            //      sparse  full        sparse          bitmap
            //      sparse  full        sparse          full  
            //      sparse  full        bitmap          sparse
            //      bitmap  full        bitmap          bitmap
            //      bitmap  full        bitmap          full  
            //      sparse  full        full            sparse
            //      bitmap  full        full            bitmap
            //      bitmap  full        full            full    (use GB_add)

            // The mask is very efficient to use in the case, when C is sparse.

            if (A_is_sparse_or_hyper || B_is_sparse_or_hyper)
            { 
                // C<M>=A.*B with A or B sparse/hyper, M bitmap/full, C sparse
                C_sparsity = GxB_SPARSE ;
            }
            else
            { 
                // C<M>=A.*B with A, B, and M bitmap/full, C bitmap
                C_sparsity = GxB_BITMAP ;
            }
        }

    }
    else // Mask_comp
    {

        //      ------------------------------------------
        //      C       <!M>=       A       .*      B
        //      ------------------------------------------
        //      sparse  sparse      sparse          sparse  (mask later)
        //      sparse  sparse      sparse          bitmap  (mask later)
        //      sparse  sparse      sparse          full    (mask later)
        //      sparse  sparse      bitmap          sparse  (mask later)
        //      bitmap  sparse      bitmap          bitmap
        //      bitmap  sparse      bitmap          full  
        //      sparse  sparse      full            sparse  (mask later)
        //      bitmap  sparse      full            bitmap
        //      bitmap  sparse      full            full    (use GB_add)

        //      ------------------------------------------
        //      C      <!M> =       A       .*      B
        //      ------------------------------------------
        //      sparse  bitmap      sparse          sparse
        //      sparse  bitmap      sparse          bitmap
        //      sparse  bitmap      sparse          full  
        //      sparse  bitmap      bitmap          sparse
        //      bitmap  bitmap      bitmap          bitmap
        //      bitmap  bitmap      bitmap          full  
        //      sparse  bitmap      full            sparse
        //      bitmap  bitmap      full            bitmap
        //      bitmap  bitmap      full            full    (use GB_add)

        //      ------------------------------------------
        //      C      <!M> =       A       .*      B
        //      ------------------------------------------
        //      sparse  full        sparse          sparse
        //      sparse  full        sparse          bitmap
        //      sparse  full        sparse          full  
        //      sparse  full        bitmap          sparse
        //      bitmap  full        bitmap          bitmap
        //      bitmap  full        bitmap          full  
        //      sparse  full        full            sparse
        //      bitmap  full        full            bitmap
        //      bitmap  full        full            full    (use GB_add)

        // GB_emult where C is sparse/hypersparse and C<!M>=A.*B:
        // Do not use a complemented mask in this case.  Do it later.

        if (A_is_sparse_or_hyper || B_is_sparse_or_hyper)
        { 
            // C<!M>=A.*B with A or B sparse/hyper, C sparse
            C_sparsity = GxB_SPARSE ;
            (*apply_mask) = !M_is_sparse_or_hyper ;
        }
        else
        { 
            // C<!M>=A.*B with A and B bitmap/full, C bitmap
            C_sparsity = GxB_BITMAP ;
        }
    }

    return (C_sparsity) ;
}


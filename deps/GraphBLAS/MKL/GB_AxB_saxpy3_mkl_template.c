//------------------------------------------------------------------------------
// GB_AxB_saxpy3_mkl_template: compute C=A*B, C<M>=A*B, or C<!M>=A*B with MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    bool use_mkl = (Context == NULL) ? false : Context->use_mkl ;

    // Note that GB_AxB_saxpy3 computes C=A*B where A and B treated as if CSC,
    // but MKL views the matrices as CSR.  So they are flipped below:

    #if GB_HAS_MKL_GRAPH

    if (use_mkl)
    {
        info = GB_AxB_saxpy3_mkl (
            Chandle,            // output matrix to construct
            M,                  // input mask M (may be NULL)
            Mask_comp,          // true if M is complemented
            Mask_struct,        // true if M is structural
            B,                  // first input matrix
            A,                  // second input matrix
            semiring,           // semiring that defines C=A*B
            !flipxy,            // true if multiply operator is flipped
            mask_applied,       // if true, then mask was applied
            Context) ;
  
        if (info != GrB_NO_VALUE)
        {
            // MKL_graph supports this semiring, and has either computed C=A*B,
            // C<M>=A*B, or C<!M>=A*B, or has failed.
            return (info) ;
        }
  
        // If MKL_graph doesn't support this semiring, it returns GrB_NO_VALUE,
        // so fall through to use GraphBLAS, below.
    }
    #endif


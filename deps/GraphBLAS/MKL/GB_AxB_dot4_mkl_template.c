//------------------------------------------------------------------------------
// GB_AxB_dot4_mkl_template: compute C+=A'*B in-place, with MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    bool use_mkl = (Context == NULL) ? false : Context->use_mkl ;

    // Note that GB_AxB_dot4 computes C+=A'*B where A and B treated as if CSC,
    // but MKL views the matrices as CSR.  MKL only handles the case when B
    // is a dense vector in mkl_graph_mxv, and A' in CSC format is the same
    // as A in CSR.

    #if GB_HAS_MKL_GRAPH

    if (use_mkl &&
        (semiring == GrB_PLUS_TIMES_SEMIRING_FP32 ||
         semiring == GxB_PLUS_SECOND_FP32) && GB_VECTOR_OK (C)
        && GB_is_dense (C) && GB_is_dense (B) && GB_VECTOR_OK (B) && !flipxy
        && !GB_IS_HYPERSPARSE (A)
        && !GB_IS_BITMAP (C) && !GB_IS_BITMAP (A) && !GB_IS_BITMAP (B))
    {
  
        info = // GrB_NO_VALUE ;
        #if 1
        GB_AxB_dot4_mkl (
            (GrB_Vector) C,     // input/output (now a vector)
            A,                  // first input matrix
            (GrB_Vector) B,     // second input (now a vector)
            semiring,           // semiring that defines C=A*B
            Context) ;
        #endif
  
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


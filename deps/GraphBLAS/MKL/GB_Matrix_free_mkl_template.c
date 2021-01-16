//------------------------------------------------------------------------------
// GB_Matrix_free_mkl_template: free a GrB_Matrix/GrB_Vector when using MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    // free the MKL optimization, if it exists
    #if GB_HAS_MKL
    GB_MKL_GRAPH_MATRIX_DESTROY (A->mkl) ;
    #endif


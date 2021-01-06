//------------------------------------------------------------------------------
// GB_Test_init_mkl_template: initialize GraphBLAS for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    bool use_mkl ;
    GxB_Global_Option_get_(GxB_MKL, &use_mkl) ;
    pargout [14] = mxCreateLogicalScalar (use_mkl) ;


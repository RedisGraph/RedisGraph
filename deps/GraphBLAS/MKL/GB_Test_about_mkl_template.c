//------------------------------------------------------------------------------
// GB_Test_about_mkl_template.c: testing with MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    int use_mkl = -99 ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_MKL, &use_mkl)) ;
    printf ("MKL control: %d\n", use_mkl) ;

    OK (GxB_Global_Option_set_(GxB_GLOBAL_MKL, true)) ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_MKL, &use_mkl)) ;
    CHECK (use_mkl == true) ;

    OK (GxB_Global_Option_set_(GxB_GLOBAL_MKL, false)) ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_MKL, &use_mkl)) ;
    CHECK (use_mkl == false) ;

    GxB_Global_Option_get_(GxB_MKL, &use_mkl) ;
    printf ("use mkl: %d\n", use_mkl) ;

    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_Desc_set (desc, GxB_MKL, false)) ;
    OK (GxB_Desc_get (desc, GxB_MKL, &use_mkl)) ;
    CHECK (use_mkl == false) ;
    GrB_Descriptor_free_(&desc) ;


//------------------------------------------------------------------------------
// GB_Descriptor_get_mkl_template: get the status of a descriptor for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    bool use_mkl = false ;
    if (desc != NULL)
    {
        use_mkl = desc->use_mkl ;
    }
    Context->use_mkl = use_mkl ;


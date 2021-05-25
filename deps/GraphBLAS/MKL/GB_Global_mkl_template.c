//------------------------------------------------------------------------------
// GB_Global_mkl_template: global values in GraphBLAS for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_use_mkl_set (bool use_mkl)
{ 
    GB_Global.use_mkl = use_mkl ;
}

GB_PUBLIC
bool GB_Global_use_mkl_get (void)
{ 
    return (GB_Global.use_mkl) ;
}


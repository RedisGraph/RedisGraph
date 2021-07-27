//------------------------------------------------------------------------------
// GB_stringify_sparsity: determine the sparsity status of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_stringify_sparsity: define macros for sparsity structure
//------------------------------------------------------------------------------

void GB_stringify_sparsity  // construct macros for sparsity structure
(
    // output:
    char *sparsity_macros,  // macros that define the sparsity structure
    // intput:
    char *matrix_name,      // "C", "M", "A", or "B"
    int A_sparsity          // GxB_SPARSE, GxB_HYPERSPARSE, GxB_BITMAP, GxB_FULL
)
{

    int ecode ;
    GB_enumify_sparsity (&ecode, A_sparsity) ;
    GB_macrofy_sparsity (sparsity_macros, matrix_name, ecode) ;
}

//------------------------------------------------------------------------------
// GB_enumify_sparsity: enumerate the sparsity structure of a matrix
//------------------------------------------------------------------------------

void GB_enumify_sparsity    // enumerate the sparsity structure of a matrix
(
    // output:
    int *ecode,             // enumerated sparsity structure
    // input:
    int A_sparsity          // GxB_SPARSE, GxB_HYPERSPARSE, GxB_BITMAP, GxB_FULL
)
{

    if (A_sparsity == GxB_SPARSE || A_sparsity == 0)
    {
        e = 0 ;
    }
    else if (A_sparsity == GxB_HYPERSPARSE)
    { 
        e = 1 ;
    }
    else if (A_sparsity == GxB_BITMAP)
    { 
        e = 2 ;
    }
    else if (A_sparsity == GxB_FULL)
    { 
        e = 3 ;
    }
    else
    { 
        e = 0 ; // if A is NULL, pretend it's sparse
    }
    (*ecode) = e ;
}

//------------------------------------------------------------------------------
// GB_macrofy_sparsity: define macro for the sparsity structure of a matrix
//------------------------------------------------------------------------------

void GB_macrofy_sparsity    // construct macros for sparsity structure
(
    // output:
    char *sparsity_macros,  // macros that define the sparsity structure
    // input:
    char *matrix_name,      // "C", "M", "A", or "B"
    int ecode
)
{

    switch (ecode)
    {

        case 0 :    // sparse
            snprintf (sparsity_macros, GB_CUDA_STRLEN,
                "#define GB_%s_IS_SPARSE 1\n"
                "#define GB_%s_IS_HYPER  0\n"
                "#define GB_%s_IS_BITMAP 0\n"
                "#define GB_%s_IS_FULL   0\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        case 1 :    // hypersparse
            snprintf (sparsity_macros, GB_CUDA_STRLEN,
                "#define GB_%s_IS_SPARSE 0\n"
                "#define GB_%s_IS_HYPER  1\n"
                "#define GB_%s_IS_BITMAP 0\n"
                "#define GB_%s_IS_FULL   0\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        case 2 :    // bitmap
            snprintf (sparsity_macros, GB_CUDA_STRLEN,
                "#define GB_%s_IS_SPARSE 0\n"
                "#define GB_%s_IS_HYPER  0\n"
                "#define GB_%s_IS_BITMAP 1\n"
                "#define GB_%s_IS_FULL   0\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        case 3 :    // full
            snprintf (sparsity_macros, GB_CUDA_STRLEN,
                "#define GB_%s_IS_SPARSE 0\n"
                "#define GB_%s_IS_HYPER  0\n"
                "#define GB_%s_IS_BITMAP 0\n"
                "#define GB_%s_IS_FULL   1\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        default :
    }
}


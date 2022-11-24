//------------------------------------------------------------------------------
// GB_macrofy_sparsity: define macro for the sparsity structure of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_sparsity    // construct macros for sparsity structure
(
    // input:
    FILE *fp,
    char *matrix_name,      // "C", "M", "A", or "B"
    int ecode
)
{

    switch (ecode)
    {

        case 0 :    // hypersparse
            fprintf ( fp,
                "#define GB_%s_IS_HYPER  1\n"
                "#define GB_%s_IS_SPARSE 0\n"
                "#define GB_%s_IS_BITMAP 0\n"
                "#define GB_%s_IS_FULL   0\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        case 1 :    // sparse
            fprintf ( fp, 
                "#define GB_%s_IS_HYPER  0\n"
                "#define GB_%s_IS_SPARSE 1\n"
                "#define GB_%s_IS_BITMAP 0\n"
                "#define GB_%s_IS_FULL   0\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        case 2 :    // bitmap
            fprintf ( fp, 
                "#define GB_%s_IS_HYPER  0\n"
                "#define GB_%s_IS_SPARSE 0\n"
                "#define GB_%s_IS_BITMAP 1\n"
                "#define GB_%s_IS_FULL   0\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        case 3 :    // full
            fprintf ( fp, 
                "#define GB_%s_IS_HYPER  0\n"
                "#define GB_%s_IS_SPARSE 0\n"
                "#define GB_%s_IS_BITMAP 0\n"
                "#define GB_%s_IS_FULL   1\n",
                matrix_name, matrix_name, matrix_name, matrix_name) ;
            break ;

        default :
            break ;
    }
}


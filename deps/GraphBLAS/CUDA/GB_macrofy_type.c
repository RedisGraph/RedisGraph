//------------------------------------------------------------------------------
// GB_macrofy_type: define macro for the type a matrix or operator input/output
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_type        // construct macros for type
(
    FILE *fp,
    // input:
    char *what,             // "C", "M", "A", "B", "X", "Y", "Z"
    GrB_Type type
)
{

    fprintf (fp, "#define GB_%s_TYPENAME %s\n", what, type->name) ;
    fprintf (fp, "#define GB_%s_TYPEDEFN \n", what) ;
    if (type->defn != NULL)
    {
        fprintf (fp, "%s\n", type->defn) ;
    }
}


//------------------------------------------------------------------------------
// GB_debugify_reduce: create the header file for a reduction problem
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_debugify_reduce     // enumerate and macrofy a GrB_reduce problem
(
    // input:
    GrB_Monoid monoid,      // the monoid to enumify
    GrB_Matrix A
)
{

    uint64_t rcode ;        // unique encoding of the entire problem

    GB_enumify_reduce (&rcode, monoid, A) ;

    char reduce_name [256 + 2 * GxB_MAX_NAME_LEN] ;
    GB_namify_problem (reduce_name, rcode,
        monoid->op->name,
        NULL,
        monoid->op->ztype->name,
        A->type->name,
        NULL,
        NULL,
        NULL,
        NULL) ;

    char filename [512 + 2 * GxB_MAX_NAME_LEN] ;
    sprintf (filename, "/tmp/GB_reduce_%s.h", reduce_name) ;
    FILE *fp = fopen (filename, "w") ;

    GB_macrofy_reduce (fp, rcode, monoid, A->type) ;
    fclose (fp) ;
} 


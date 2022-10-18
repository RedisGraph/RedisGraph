//------------------------------------------------------------------------------
// GB_macrofy_reduce: construct all macros for a reduction to scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_reduce      // construct all macros for GrB_reduce to scalar
(
    FILE *fp,               // target file to write, already open
    // input:
    uint64_t rcode,         // encoded problem
    GrB_Monoid monoid,      // monoid to macrofy
    GrB_Type atype          // type of the A matrix to reduce
)
{ 

    //--------------------------------------------------------------------------
    // extract the reduction rcode
    //--------------------------------------------------------------------------

    // monoid
    int red_ecode   = GB_RSHIFT (rcode, 20, 5) ;
    int id_ecode    = GB_RSHIFT (rcode, 15, 5) ;
    int term_ecode  = GB_RSHIFT (rcode, 10, 5) ;
    bool is_term    = (term_ecode < 30) ;

    // type of the monoid
    int zcode       = GB_RSHIFT (rcode, 6, 4) ;

    // type of A
    int acode       = GB_RSHIFT (rcode, 2, 4) ;

    // format of A
    int asparsity   = GB_RSHIFT (rcode, 0, 2) ;

    //--------------------------------------------------------------------------
    // construct the macros for the type name
    //--------------------------------------------------------------------------

    fprintf (fp, "// GB_reduce_%016" PRIX64 ".h (%s %s A: %s)\n", rcode,
        monoid->op->name, monoid->op->ztype->name, atype->name) ;

    //--------------------------------------------------------------------------
    // construct the typedefs
    //--------------------------------------------------------------------------

    GB_macrofy_types (fp, NULL, atype->defn, NULL,
        NULL, NULL, monoid->op->ztype->defn) ;

    //--------------------------------------------------------------------------
    // construct the macros for the type names
    //--------------------------------------------------------------------------

    fprintf (fp, "\n// monoid type:\n") ;
    fprintf (fp, "#define GB_Z_TYPENAME %s\n", monoid->op->ztype->name) ;

    //--------------------------------------------------------------------------
    // construct the monoid macros
    //--------------------------------------------------------------------------

    fprintf (fp, "\n// reduction monoid:\n") ;
    GB_macrofy_monoid (fp, red_ecode, id_ecode, term_ecode, monoid, false) ;

    //--------------------------------------------------------------------------
    // construct the macros for A
    //--------------------------------------------------------------------------

    fprintf (fp, "\n// A matrix:\n") ;
    GB_macrofy_sparsity (fp, "A", asparsity) ;
    fprintf (fp, "#define GB_A_TYPENAME %s\n", atype->name) ;
}


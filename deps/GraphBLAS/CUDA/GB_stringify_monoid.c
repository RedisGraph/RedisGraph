//------------------------------------------------------------------------------
// GB_stringify_monoid: build strings for a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Construct a string defining all macros for a monoid, and its name.
// User-defined types are not handled.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_enumify_monoid: build strings for a monoid
//------------------------------------------------------------------------------

void GB_enumify_monoid  // enumerate a monoid
(
    // outputs:
    int *add_ecode,     // binary op as an enum
    int *id_ecode,      // identity value as an enum
    int *term_ecode,    // terminal value as an enum
    // inputs:
    int add_opcode,     // must be a built-in binary operator from a monoid
    int zcode           // type of the monoid (x, y, and z)
)
{

    printf("Calling enumify binop\n");
    GB_enumify_binop (add_ecode, add_opcode, zcode, false) ;
    ASSERT (*add_ecode < 32) ;
    printf("Calling enumify identity\n");
    GB_enumify_identity (id_ecode, add_opcode, zcode) ;
    bool is_term ;
    printf("Calling enumify terminal\n");
    GB_enumify_terminal (&is_term, term_ecode, add_opcode, zcode) ;
    printf("Done enumify monoid\n");
}

//------------------------------------------------------------------------------
// GB_macrofy_monoid: build macros for a monoid
//------------------------------------------------------------------------------

void GB_macrofy_monoid  // construct the macros for a monoid
(
    // inputs:
    FILE *fp,           // File to write macros, assumed open already
    int add_ecode,      // binary op as an enum
    int id_ecode,       // identity value as an enum
    int term_ecode,     // terminal value as an enum (< 30 is terminal)
    bool is_term
)
{

    const char *s ;

    GB_charify_binop (&s, add_ecode) ;
    GB_macrofy_binop ( fp, "GB_ADD", s, false) ;

    const char *id_value = GB_charify_identity_or_terminal (id_ecode) ;
    GB_macrofy_identity ( fp, id_value) ;

    char texpr [GB_CUDA_STRLEN+1] ;
    char tstmt [GB_CUDA_STRLEN+1] ;

    // convert ecode and is_term to strings
    const char *term_value = GB_charify_identity_or_terminal (term_ecode) ;
    GB_charify_terminal_expression (texpr, term_value, is_term, term_ecode) ;
    GB_charify_terminal_statement  (tstmt, term_value, is_term, term_ecode) ;

    // convert strings to macros
    GB_macrofy_terminal_expression ( fp,
        "GB_TERMINAL_CONDITION", texpr) ;
    GB_macrofy_terminal_statement ( fp,
        "GB_IF_TERMINAL_BREAK", tstmt) ;
}


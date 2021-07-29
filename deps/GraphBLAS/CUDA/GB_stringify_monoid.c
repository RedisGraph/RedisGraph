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

    GB_enumify_binop (add_ecode, add_opcode, zcode, false) ;
    ASSERT (*add_ecode < 32) ;
    GB_enumify_identity (id_ecode, add_opcode, zcode) ;
    bool is_term ;
    GB_enumify_terminal (&is_term, term_ecode, add_opcode, zcode) ;
}

//------------------------------------------------------------------------------
// GB_macrofy_monoid: build macros for a monoid
//------------------------------------------------------------------------------

void GB_macrofy_monoid  // construct the macros for a monoid
(
    // outputs:
    char *add_macro,                    // additive binary operator
    char *identity_macro,               // identity value
    char *terminal_expression_macro,    // terminal expr for "if (expr) ..."
    char *terminal_statement_macro,     // break statement
    // inputs:
    int add_ecode,      // binary op as an enum
    int id_ecode,       // identity value as an enum
    int term_ecode      // terminal value as an enum (< 30 is terminal)
)
{

    char s [GB_CUDA_STRLEN+1] ;

    GB_charify_binop (&s, add_ecode) ;
    GB_macrofy_binop (add_macro, "GB_ADD", s) ;

    GB_charify_identity_or_terminal (&s, id_ecode) ;
    GB_macrofy_identity (identity_macro, s) ;

    char texpr [GB_CUDA_STRLEN+1] ;
    char tstmt [GB_CUDA_STRLEN+1] ;

    // convert ecode and is_term to strings
    GB_charify_identity_or_terminal (&s, ecode) ;
    GB_charify_terminal_expression (texpr, s, is_term, ecode) ;
    GB_charify_terminal_statement  (tstmt, s, is_term, ecode) ;

    // convert strings to macros
    GB_macrofy_terminal_expression (terminal_expression_macro,
        "GB_TERMINAL_CONDITION", texpr) ;
    GB_macrofy_terminal_statement (terminal_statement_macro,
        "GB_IF_TERMINAL_BREAK", tstmt) ;
}


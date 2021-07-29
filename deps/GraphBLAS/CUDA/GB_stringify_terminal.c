//------------------------------------------------------------------------------
// GB_stringify_terminal: convert terminal condition into a string or enum
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The terminal_expression_macro(cij) should return true if the value of
// cij has reached its terminal value, or false otherwise.  If the monoid is
// not terminal, then the macro should always return false.  The ANY monoid
// should always return true.

// The terminal_statement_macro is a macro containing a full statement.  If the
// monoid is never terminal, it becomes the empty statement.  Otherwise,
// it checks the terminal condition and does a "break" if true.  The statement
// has no trailing semicolon.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_stringify_terminal: macros for terminal value and condition
//------------------------------------------------------------------------------

void GB_stringify_terminal         // return strings to check terminal
(
    // outputs:
    bool *is_monoid_terminal,           // true if monoid is terminal
    char *terminal_expression_macro,    // #define for terminal expression macro
    char *terminal_statement_macro,     // #define for terminal statement macro
    // inputs:
    const char *terminal_expression_macro_name,     // name of expression macro
    const char *terminal_statement_macro_name,      // name of statement macro
    GB_Opcode opcode,    // must be a built-in binary operator from a monoid
    GB_Type_code zcode   // type code of the binary operator
)
{

    char *terminal_value ;
    char terminal_expression [GB_CUDA_STRLEN+1] ;
    char terminal_statement  [GB_CUDA_STRLEN+1] ;
    int ecode ;

    // get ecode and bool (is_monoid_terminal) from the opcode and zcode
    GB_enumify_terminal (&ecode, opcode, zcode) ;
    (*is_monoid_terminal) = (ecode <= 29) ;

    // convert ecode and is_monoid_terminal to strings
    GB_charify_identity_or_terminal (&terminal_value, ecode) ;
    GB_charify_terminal_expression (terminal_expression,
        terminal_value, is_monoid_terminal, ecode) ;
    GB_charify_terminal_statement (terminal_statement,
        terminal_value, is_monoid_terminal, ecode) ;

    // convert strings to macros
    GB_macrofy_terminal_expression (terminal_expression_macro,
        terminal_expression_macro_name, terminal_expression) ;
    GB_macrofy_terminal_statement (terminal_statement_macro,
        terminal_statement_macro_name, terminal_statement) ;
}

//------------------------------------------------------------------------------
// GB_enumify_terminal: return enum of terminal value
//------------------------------------------------------------------------------

void GB_enumify_terminal       // return enum of terminal value
(
    // output:
    int *ecode,                 // enumerated terminal, 0 to 31 (-1 if fail)
    // input:
    GB_Opcode opcode,           // built-in binary opcode of a monoid
    GB_Type_code zcode          // type code used in the opcode we want
)
{

    int e = -1 ;

    switch (opcode)
    {

        case GB_PLUS_opcode :

            if (zcode == GB_BOOL_code)
            {
                e = 2 ;                 // true (boolean OR)
            }
            else
            {
                e = 31 ;                // builtin with no terminal value
            }
            break ;

        case GB_TIMES_opcode :

            switch (zcode)
            {
                case GB_BOOL_code   : 
                    e = 3 ;             // false (boolean AND)
                    break ;
                case GB_INT8_code   :
                case GB_INT16_code  :
                case GB_INT32_code  :
                case GB_INT64_code  :
                case GB_UINT8_code  :
                case GB_UINT16_code :
                case GB_UINT32_code :
                case GB_UINT64_code :
                    e = 0 ;             // 0
                    break ;
                default :
                    e = 31 ;            // builtin with no terminal value
                    break ;
            }
            break ;

        case GB_LOR_opcode      : 

                e = 2 ;                 // true
                break ;

        case GB_LAND_opcode     : 

                e = 3 ;                 // false
                break ;

        case GB_MIN_opcode :

            switch (zcode)
            {
                case GB_BOOL_code   : e =  3 ; break ; // false
                case GB_INT8_code   : e = 13 ; break ; // INT8_MIN
                case GB_INT16_code  : e = 14 ; break ; // INT16_MIN
                case GB_INT32_code  : e = 15 ; break ; // INT32_MIN
                case GB_INT64_code  : e = 16 ; break ; // INT64_MIN
                case GB_UINT8_code  :
                case GB_UINT16_code : 
                case GB_UINT32_code : 
                case GB_UINT64_code : e =  0 ; break ; // 0
                case GB_FP32_code   : 
                case GB_FP64_code   : e = 17 ; break ; // -INFINITY
                default             : e = -1 ; break ;
            }
            break ;

        case GB_MAX_opcode :

            switch (zcode)
            {
                case GB_BOOL_code   : e =  2 ; break ; // true
                case GB_INT8_code   : e =  4 ; break ; // INT8_MAX
                case GB_INT16_code  : e =  5 ; break ; // INT16_MAX
                case GB_INT32_code  : e =  6 ; break ; // INT32_MAX
                case GB_INT64_code  : e =  7 ; break ; // INT64_MAX
                case GB_UINT8_code  : e =  8 ; break ; // UINT8_MAX
                case GB_UINT16_code : e =  9 ; break ; // UINT16_MAX
                case GB_UINT32_code : e = 10 ; break ; // UINT32_MAX
                case GB_UINT64_code : e = 11 ; break ; // UINT64_MAX
                case GB_FP32_code   : 
                case GB_FP64_code   : e = 12 ; break ; // INFINITY
                default             : e = -1 ; break ;
            }
            break ;

        case GB_ANY_opcode :

            e = 18 ;                    // no specific terminal value
            break ;

        case GB_LXOR_opcode     :
        // case GB_LXNOR_opcode :
        case GB_EQ_opcode       :
        default                 :

            e = 31 ;                    // builtin with no terminal value
            break ;

        case GB_USER_opcode :

    }

    (*ecode) = e ;
}

//------------------------------------------------------------------------------
// GB_charify_terminal_expression: string to evaluate terminal expression
//------------------------------------------------------------------------------

void GB_charify_terminal_expression    // string for terminal expression
(
    // output:
    char *terminal_expression,          // string with terminal expression
    // input:
    char *terminal_string,              // string with terminal value
    bool is_monoid_terminal,            // true if monoid is terminal
    int ecode                           // ecode of monoid operator
)
{

    if (is_monoid_terminal)
    {
        // the monoid is terminal
        if (ecode == 18)
        {
            // ANY monoid: terminal expression is always true
            snprintf (terminal_expression, GB_CUDA_STRLEN, "(true)") ;
        }
        else
        {
            // typical terminal monoids: check if C(i,j) has reached its
            // terminal value
            snprintf (terminal_expression, GB_CUDA_STRLEN,
                "((cij) == %s)", terminal_string) ;
        }
    }
    else
    {
        // the monoid is not terminal: the expression is always false
        snprintf (terminal_expression, GB_CUDA_STRLEN, "(false)") ;
    }
}

//------------------------------------------------------------------------------
// GB_charify_terminal_statement: string for terminal statement
//------------------------------------------------------------------------------

void GB_charify_terminal_statement // string for terminal statement
(
    // output:
    char *terminal_statement,           // string with terminal statement
    // input:
    char *terminal_string,              // string with terminal value
    bool is_monoid_terminal,            // true if monoid is terminal
    int ecode                           // ecode of monoid operator
)
{

    if (is_monoid_terminal)
    {
        // the monoid is terminal
        if (ecode == 18)
        {
            // ANY monoid: always break
            snprintf (terminal_statement, GB_CUDA_STRLEN, "break") ;
        }
        else
        {
            // typical terminal monoids: break if C(i,j) has reached its
            // terminal value
            snprintf (terminal_statement, GB_CUDA_STRLEN,
                "if ((cij) == %s) break", terminal_string) ;
        }
    }
    else
    {
        // the monoid is not terminal: the terminal statement is empty
        snprintf (terminal_statement, GB_CUDA_STRLEN, "") ;
    }
}

//------------------------------------------------------------------------------
// GB_macrofy_terminal_expression: macro for terminal expression
//------------------------------------------------------------------------------

void GB_macrofy_terminal_expression    // macro for terminal expression
(
    // output:
    char *terminal_expression_macro,
    // intput:
    const char *terminal_expression_macro_name,
    const char *terminal_expression
)
{

    snprintf (terminal_expression_macro, GB_CUDA_STRLEN,
        "#define %s(cij) %s",
        terminal_expression_macro_name, terminal_expression) ;
}

//------------------------------------------------------------------------------
// GB_macrofy_terminal_statement: macro for terminal statement
//------------------------------------------------------------------------------

void GB_macrofy_terminal_statement     // macro for terminal statement
(
    // output:
    char *terminal_statement_macro,
    // intput:
    const char *terminal_statement_macro_name,
    const char *terminal_statement
)
{

    snprintf (terminal_statement_macro, GB_CUDA_STRLEN,
        "#define %s %s",
        terminal_statement_macro_name, terminal_statement) ;
}


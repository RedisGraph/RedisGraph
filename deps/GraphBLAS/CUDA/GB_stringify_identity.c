//------------------------------------------------------------------------------
// GB_stringify_identity: return string or enum for identity value
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_stringify_identity: macro for identity value
//------------------------------------------------------------------------------

void GB_stringify_identity     // return string for identity value
(
    // input:
    FILE *fp,               // File to write macros, assumed open already
    GB_Opcode opcode,       // must be a built-in binary operator from a monoid
    GB_Type_code zcode      // type code of the binary operator
)
{

    int ecode ;

    // get ecode from the opcode and zcode
    GB_enumify_identity (&ecode, opcode, zcode) ;

    // convert ecode to string
    const char *identity_value = GB_charify_identity_or_terminal (ecode) ;

    // convert string to macro
    GB_macrofy_identity ( fp, identity_value) ;
}

//------------------------------------------------------------------------------
// GB_enumify_identity: return ecode for identity value of an operator 
//------------------------------------------------------------------------------

void GB_enumify_identity       // return enum of identity value
(
    // output:
    int *ecode,             // enumerated identity, 0 to 17 (-1 if fail)
    // inputs:
    GB_Opcode opcode,       // built-in binary opcode of a monoid
    GB_Type_code zcode      // type code used in the opcode we want
)
{

    int e = -1 ;

    switch (opcode)
    {

        case GB_PLUS_binop_code     : e = 0 ; break ; // 0

        case GB_TIMES_binop_code    : e = 1 ; break ; // 1

        case GB_LAND_binop_code     : 
        // case GB_LXNOR_binop_code : 
        case GB_EQ_binop_code       : 
            e = (zcode == GB_BOOL_code) ? 2 : (-1) ; break ; // true

        case GB_LOR_binop_code      : 
        case GB_LXOR_binop_code     : 
            e = (zcode == GB_BOOL_code) ? 3 : (-1) ; break ; // false

        case GB_MIN_binop_code :

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

        case GB_MAX_binop_code :

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

        case GB_ANY_binop_code      : e = 0 ; break ; // 0

        // identity/terminal values for user-defined monoids must be provided
        // by an additional string.  This value is a place-holder to indicate
        // that the additional user-provided string must be used.
        case GB_USER_binop_code     : e = 31 ; break ;

        default                     : e = -1 ; break ; // invalid op or type
    }

    (*ecode) = e ;
}

//------------------------------------------------------------------------------
// GB_charify_identity_or_terminal: string for identity/terminal value
//------------------------------------------------------------------------------

const char *GB_charify_identity_or_terminal // return string encoding the value
(
    // input:
    int ecode                   // enumerated identity/terminal value
)
{

    const char *f ;

    switch (ecode)
    {

        //----------------------------------------------------------------------
        // for identity values and terminal values for terminal monoids
        //----------------------------------------------------------------------

        case  0 : f = "0"           ; break ;
        case  1 : f = "1"           ; break ;
        case  2 : f = "true"        ; break ;
        case  3 : f = "false"       ; break ;
        case  4 : f = "INT8_MAX"    ; break ;
        case  5 : f = "INT16_MAX"   ; break ;
        case  6 : f = "INT32_MAX"   ; break ;
        case  7 : f = "INT64_MAX"   ; break ;
        case  8 : f = "UINT8_MAX"   ; break ;
        case  9 : f = "UINT16_MAX"  ; break ;
        case 10 : f = "UINT32_MAX"  ; break ;
        case 11 : f = "UINT64_MAX"  ; break ;
        case 12 : f = "INFINITY"    ; break ;
        case 13 : f = "INT8_MIN"    ; break ;
        case 14 : f = "INT16_MIN"   ; break ;
        case 15 : f = "INT32_MIN"   ; break ;
        case 16 : f = "INT64_MIN"   ; break ;
        case 17 : f = "-INFINITY"   ; break ;
        case 18 : f = "0"           ; break ;       // for the ANY monoid only

        // ecodes 19 to 28 are reserved for future use, for terminal values

        // user-defined terminal monoid
        case 29 : f = "(user provided, terminal)" ; break ;

        //----------------------------------------------------------------------
        // for non-terminal monoids
        //----------------------------------------------------------------------

        // user-defined non-terminal monoids
        case 30 : f = "(user provided, not terminal)" ; break ;

        // built-in non-terminal monoids
        case 31 : f = "(built in, not terminal)" ; break ;

        default : f = NULL ;        ; break ;
    }

    return (f) ;
}

//------------------------------------------------------------------------------
// GB_macrofy_identity: convert a value string to a macro
//------------------------------------------------------------------------------

void GB_macrofy_identity
(
    // input:
    FILE *fp,                   // File to write macros, assumed open already
    const char *value_string    // string defining the identity value
)
{
    fprintf ( fp, "#define GB_IDENTITY (%s)\n", value_string) ;
}


//------------------------------------------------------------------------------
// GB_charify_identity_or_terminal: string for identity/terminal value
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

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

        case  0 : f = "0"                   ; break ;
        case  1 : f = "1"                   ; break ;
        case  2 : f = "true"                ; break ;
        case  3 : f = "false"               ; break ;
        case  4 : f = "INT8_MAX"            ; break ;
        case  5 : f = "INT16_MAX"           ; break ;
        case  6 : f = "INT32_MAX"           ; break ;
        case  7 : f = "INT64_MAX"           ; break ;
        case  8 : f = "UINT8_MAX"           ; break ;
        case  9 : f = "UINT16_MAX"          ; break ;
        case 10 : f = "UINT32_MAX"          ; break ;
        case 11 : f = "UINT64_MAX"          ; break ;
        case 12 : f = "INFINITY"            ; break ;
        case 13 : f = "INT8_MIN"            ; break ;
        case 14 : f = "INT16_MIN"           ; break ;
        case 15 : f = "INT32_MIN"           ; break ;
        case 16 : f = "INT64_MIN"           ; break ;
        case 17 : f = "-INFINITY"           ; break ;
        case 18 : f = "0"                   ; break ; // for ANY monoid only
        case 19 : f = "0xFF"                ; break ;
        case 20 : f = "0xFFFF"              ; break ;
        case 21 : f = "0xFFFFFFFF"          ; break ;
        case 22 : f = "0xFFFFFFFFFFFFFFFF"  ; break ;

        // ecodes 19 to 28 are reserved for future use

        // user-defined monoid (terminal or non-terminal) or
        // built-in non-terminal monoid
        default : f = "" ;                  ; break ;
    }

    return (f) ;
}


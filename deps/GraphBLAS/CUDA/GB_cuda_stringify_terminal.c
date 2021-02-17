// SPDX-License-Identifier: Apache-2.0
//------------------------------------------------------------------------------
// GB_cuda_stringify_terminal: string to check terminal condition
//------------------------------------------------------------------------------

// The macro_condition_name(cij) should return true if the value of cij has
// reached its terminal value, or false otherwise.  If the monoid is not
// terminal, then the macro should always return false.  The ANY monoid
// should always return true.

// The macro_statement_name is a macro containing a full statement.  If the
// monoid is never terminal, it becomes the empty statement (";").  Otherwise,
// it checks the terminal condition and does a "break" if true.

#include "GB.h"
#include "GB_cuda_stringify.h"

void GB_cuda_stringify_terminal // return strings to check terminal
(
    // outputs:
    bool *is_monoid_terminal,
    char *terminal_condition,
    char *terminal_statement,
    // inputs:
    const char *macro_condition_name,
    const char *macro_statement_name,
    GB_Opcode opcode,    // must be a built-in binary operator from a monoid
    GB_Type_code zcode   // op->ztype->code
)
{

    //--------------------------------------------------------------------------
    // determine if the monoid is terminal, and find its terminal value
    //--------------------------------------------------------------------------

    bool is_terminal = false ;
    const char *f = NULL ;

    switch (opcode)
    {

        #if 0
        case GB_ANY_opcode :
            f = NULL ;
            is_terminal = true ;
            break ;
        #endif

        case GB_MIN_opcode :

            is_terminal = true ;
            switch (zcode)
            {
                case GB_BOOL_code   : f = "false" ;         break ;
                case GB_INT8_code   : f = "INT8_MIN" ;      break ;
                case GB_INT16_code  : f = "INT16_MIN" ;     break ;
                case GB_INT32_code  : f = "INT32_MIN" ;     break ;
                case GB_INT64_code  : f = "INT64_MIN" ;     break ;
                case GB_UINT8_code  : f = "0" ;             break ;
                case GB_UINT16_code : f = "0" ;             break ;
                case GB_UINT32_code : f = "0" ;             break ;
                case GB_UINT64_code : f = "0" ;             break ;
                default             : f = "(-INFINITY)" ;   break ;
            }
            break ;

        case GB_MAX_opcode :

            is_terminal = true ;
            switch (zcode)
            {
                case GB_BOOL_code   : f = "true" ;          break ;
                case GB_INT8_code   : f = "INT8_MAX" ;      break ;
                case GB_INT16_code  : f = "INT16_MAX" ;     break ;
                case GB_INT32_code  : f = "INT32_MAX" ;     break ;
                case GB_INT64_code  : f = "INT64_MAX" ;     break ;
                case GB_UINT8_code  : f = "UINT8_MAX" ;     break ;
                case GB_UINT16_code : f = "UINT16_MAX" ;    break ;
                case GB_UINT32_code : f = "UINT32_MAX" ;    break ;
                case GB_UINT64_code : f = "UINT64_MAX" ;    break ;
                default             : f = "INFINITY" ;      break ;
            }
            break ;

        case GB_PLUS_opcode :

            if (zcode == GB_BOOL_code)
            {
                f = "true" ;      // boolean OR
                is_terminal = true ;
            }
            else
            {
                f = NULL ;
                is_terminal = false ;
            }
            break ;

        case GB_TIMES_opcode :

            switch (zcode)
            {
                case GB_BOOL_code   :   // boolean AND
                case GB_INT8_code   :
                case GB_INT16_code  :
                case GB_INT32_code  :
                case GB_INT64_code  :
                case GB_UINT8_code  :
                case GB_UINT16_code :
                case GB_UINT32_code :
                case GB_UINT64_code :
                    f = "0" ;
                    is_terminal = true ;
                    break ;
                default             :
                    f = NULL ;
                    is_terminal = false ;
                    break ;
            }
            break ;

        case GB_LOR_opcode      : f = "true"  ; is_terminal = true  ; break ;
        case GB_LAND_opcode     : f = "false" ; is_terminal = true  ; break ; 

        case GB_LXOR_opcode     :
        // case GB_LXNOR_opcode :
        case GB_EQ_opcode       :
        default                 :
            // the monoid is not terminal
            f = NULL ;
            is_terminal = false ;
            break ;
    }

    //--------------------------------------------------------------------------
    // construct the macro to test the terminal condition
    //--------------------------------------------------------------------------

    if (is_terminal)
    {
        // the monoid is terminal
        if (f == NULL)
        {
            // ANY monoid
            snprintf (terminal_condition, GB_CUDA_STRLEN,
                "#define %s(cij) true", macro_condition_name) ;
            snprintf (terminal_statement, GB_CUDA_STRLEN,
                "#define %s break", macro_statement_name) ;
        }
        else
        {
            // typical terminal monoids: check if C(i,j) has reached its
            // terminal value
            snprintf (terminal_condition, GB_CUDA_STRLEN,
                "#define %s(cij) ((cij) == %s)", macro_condition_name, f) ;
            snprintf (terminal_statement, GB_CUDA_STRLEN,
                "#define %s if (%s (cij)) break",
                macro_statement_name, macro_condition_name) ;
        }
    }
    else
    {
        // the monoid is not terminal: the condition is always false
        snprintf (terminal_condition, GB_CUDA_STRLEN, "#define %s(cij) false",
            macro_condition_name) ;
        snprintf (terminal_statement, GB_CUDA_STRLEN, "#define %s",
            macro_statement_name) ;
    }

    (*is_monoid_terminal) = is_terminal ;
}


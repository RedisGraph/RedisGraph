// SPDX-License-Identifier: Apache-2.0
//------------------------------------------------------------------------------
// GB_cuda_stringify_identity: return string for identity value
//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_cuda_stringify.h"

#define  ID( x)  IDENT = (x)

void GB_cuda_stringify_identity        // return string for identity value
(
    // output:
    char *code_string,  // string with the #define macro
    // input:
    GB_Opcode opcode,     // must be a built-in binary operator from a monoid
    GB_Type_code zcode    // type code used in the opcode we want
)
{
    const char *IDENT;
    switch (opcode)
    {
        case GB_MIN_opcode :

            switch (zcode)
            {
                case GB_BOOL_code   : ID ("true") ;     // boolean AND
                case GB_INT8_code   : ID ("INT8_MAX") ;
                case GB_INT16_code  : ID ("INT16_MAX") ;
                case GB_INT32_code  : ID ("INT32_MAX") ;
                case GB_INT64_code  : ID ("INT64_MAX") ;
                case GB_UINT8_code  : ID ("UINT8_MAX") ;
                case GB_UINT16_code : ID ("UINT16_MAX") ;
                case GB_UINT32_code : ID ("UINT32_MAX") ;
                case GB_UINT64_code : ID ("UINT64_MAX") ;
                default             : ID ("INFINITY") ;
            }
            break ;

        case GB_MAX_opcode :

            switch (zcode)
            {
                case GB_BOOL_code   : ID ("false") ;    // boolean OR
                case GB_INT8_code   : ID ("INT8_MIN") ;
                case GB_INT16_code  : ID ("INT16_MIN") ;
                case GB_INT32_code  : ID ("INT32_MIN") ;
                case GB_INT64_code  : ID ("INT64_MIN") ;
                case GB_UINT8_code  : ID ("0") ;
                case GB_UINT16_code : ID ("0") ;
                case GB_UINT32_code : ID ("0") ;
                case GB_UINT64_code : ID ("0") ;
                default             : ID ("(-INFINITY)") ;
            }
            break ;

        case GB_PLUS_opcode     : ID ("0") ;
        case GB_TIMES_opcode    : ID ("1") ;
        case GB_LOR_opcode      : ID ("false") ;
        case GB_LAND_opcode     : ID ("true") ;
        case GB_LXOR_opcode     : ID ("false") ;
        // case GB_LXNOR_opcode :
        case GB_EQ_opcode       : ID ("true") ;
        // case GB_ANY_opcode   :
        default                 : ID ("0") ;
    }
    snprintf (code_string, GB_CUDA_STRLEN, "#define GB_IDENTITY (%s)", IDENT) ;

}


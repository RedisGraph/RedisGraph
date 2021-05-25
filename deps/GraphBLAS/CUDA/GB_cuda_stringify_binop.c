//SPDX-License-Identifier: Apache-2.0

#include "GB.h"
#include "GB_cuda_stringify.h"

// The binop macro generates an expression, not a full statement (there
// is no semicolon).

// for example:
// #define GB_MULT(x,y) ((x) * (y))

void GB_cuda_stringify_binop
(
    // output:
    char *code_string,  // string with the #define macro
    // input:
    const char *macro_name,   // name of macro to construct
    GB_Opcode opcode,   // opcode of GraphBLAS operator to convert into a macro
    GB_Type_code zcode  // op->ztype->code of the operator
)
{

    const char *f ;

    switch (opcode)
    {

        case GB_FIRST_opcode :    //  7: z = x

            f = "(x)" ;
            break ;

        // case GB_ANY_opcode :
        case GB_SECOND_opcode :   //  8: z = y

            f = "(y)" ;
            break ;

        case GB_MIN_opcode :      //  9: z = min(x,y)

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) && (y)" ;
                case GB_FP32_code    : f = "fminf (x,y)" ;
                case GB_FP64_code    : f = "fmin (x,y)" ;
                default              : f = "GB_IMIN (x,y)" ;
            }
            break ;

        case GB_MAX_opcode :      // 10: z = max(x,y)

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) || (y)" ;
                case GB_FP32_code    : f = "fmaxf (x,y)" ;
                case GB_FP64_code    : f = "fmax (x,y)" ;
                default              : f = "GB_IMAX (x,y)" ;
            }
            break ;

        case GB_PLUS_opcode :     // 11: z = x + y

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) || (y)" ;
                default              : f = "(x) + (y)" ;
            }
            break ;

        case GB_MINUS_opcode :    // 12: z = x - y

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) != (y)" ;
                default              : f = "(x) - (y)" ;
            }
            break ;

        case GB_RMINUS_opcode :   // 13: z = y - x

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) != (y)" ;
                default              : f = "(y) - (x)" ;
            }
            break ;

        case GB_TIMES_opcode :    // 14: z = x * y

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) && (y)" ;
                default              : f = "(x) * (y)" ;
            }
            break ;

        case GB_DIV_opcode :      // 15: z = x / y ;

            switch (zcode)
            {
                case GB_BOOL_code   : f = "(x)" ;
                case GB_INT8_code   : f = "GB_IDIV_SIGNED (x,y,8)" ;
                case GB_INT16_code  : f = "GB_IDIV_SIGNED (x,y,16)" ;
                case GB_INT32_code  : f = "GB_IDIV_SIGNED (x,y,32)" ;
                case GB_INT64_code  : f = "GB_IDIV_SIGNED (x,y,64)" ;
                case GB_UINT8_code  : f = "GB_IDIV_UNSIGNED (x,y,8)" ;
                case GB_UINT16_code : f = "GB_IDIV_UNSIGNED (x,y,16)" ;
                case GB_UINT32_code : f = "GB_IDIV_UNSIGNED (x,y,32)" ;
                case GB_UINT64_code : f = "GB_IDIV_UNSIGNED (x,y,64)" ;
                default             : f = "(x) / (y)" ;
            }
            break ;

        case GB_RDIV_opcode :      // z = y / x ;

            switch (zcode)
            {
                case GB_BOOL_code   : f = "(x)" ;
                case GB_INT8_code   : f = "GB_IDIV_SIGNED (y,x,8)" ;
                case GB_INT16_code  : f = "GB_IDIV_SIGNED (y,x,16)" ;
                case GB_INT32_code  : f = "GB_IDIV_SIGNED (y,x,32)" ;
                case GB_INT64_code  : f = "GB_IDIV_SIGNED (y,x,64)" ;
                case GB_UINT8_code  : f = "GB_IDIV_UNSIGNED (y,x,8)" ;
                case GB_UINT16_code : f = "GB_IDIV_UNSIGNED (y,x,16)" ;
                case GB_UINT32_code : f = "GB_IDIV_UNSIGNED (y,x,32)" ;
                case GB_UINT64_code : f = "GB_IDIV_UNSIGNED (y,x,64)" ;
                default             : f = "(y) / (x)" ;
            }
            break ;

        case GB_EQ_opcode :
        case GB_ISEQ_opcode :     // 17: z = (x == y)

            f = "(x) == (y)" ;
            break ;

        case GB_NE_opcode :
        case GB_ISNE_opcode :     // 18: z = (x != y)

            f = "(x) != (y)" ;
            break ;

        case GB_GT_opcode :
        case GB_ISGT_opcode :     // 19: z = (x >  y)

            f = "(x) > (y)" ;
            break ;

        case GB_LT_opcode :
        case GB_ISLT_opcode :     // 20: z = (x <  y)

            f = "(x) < (y)" ;
            break ;

        case GB_GE_opcode :
        case GB_ISGE_opcode :     // 21: z = (x >= y)

            f = "(x) >= (y)" ;
            break ;

        case GB_LE_opcode :
        case GB_ISLE_opcode :     // 22: z = (x <= y)

            f = "(x) <= (y)" ;
            break ;

        case GB_LOR_opcode :      // 23: z = (x != 0) || (y != 0)

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) || (y)" ;
                default              : f = "((x) != 0) || ((y) != 0)" ;
            }
            break ;

        case GB_LAND_opcode :     // 23: z = (x != 0) && (y != 0)

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) && (y)" ;
                default              : f = "((x) != 0) && ((y) != 0)" ;
            }
            break ;

        case GB_LXOR_opcode :     // 25: z = (x != 0) != (y != 0)

            switch (zcode)
            {
                case GB_BOOL_code    : f = "(x) != (y)" ;
                default              : f = "((x) != 0) != ((y) != 0)" ;
            }
            break ;

        // case GB_BOR_opcode : ... bitwise ops
        // x | y, etc

        // case GB_PAIR_opcode :
        default :
            
            f = "1" ;
            break ;
    }

    snprintf (code_string, GB_CUDA_STRLEN,
        "#define %s(x,y) (%s)", macro_name, f) ;
}


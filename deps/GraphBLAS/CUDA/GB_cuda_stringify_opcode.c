// SPDX-License-Identifier: Apache-2.0

#include "GB.h"
#include "GB_cuda_stringify.h"

const char *GB_cuda_stringify_opcode
(
    GB_Opcode opcode    // opcode of GraphBLAS operator
)
{

    switch (opcode)
    {
        case GB_FIRST_opcode :  return ("1st") ;
        // case GB_ANY_opcode : return ("any") ;
        case GB_SECOND_opcode : return ("2nd") ;
        case GB_MIN_opcode :    return ("min") ;
        case GB_MAX_opcode :    return ("max") ;
        case GB_PLUS_opcode :   return ("plus") ;
        case GB_MINUS_opcode :  return ("minus") ;
        case GB_RMINUS_opcode : return ("rminus") ;
        case GB_TIMES_opcode :  return ("times") ;
        case GB_DIV_opcode :    return ("div") ;
        case GB_RDIV_opcode :   return ("rdiv") ;
        case GB_EQ_opcode :     return ("eq") ;
        case GB_ISEQ_opcode :   return ("iseq") ;
        case GB_NE_opcode :     return ("ne") ;
        case GB_ISNE_opcode :   return ("isne") ;
        case GB_GT_opcode :     return ("gt") ;
        case GB_ISGT_opcode :   return ("isgt") ;
        case GB_LT_opcode :     return ("lt") ;
        case GB_ISLT_opcode :   return ("islt") ;
        case GB_GE_opcode :     return ("ge") ;
        case GB_ISGE_opcode :   return ("isge") ;
        case GB_LE_opcode :     return ("le") ;
        case GB_ISLE_opcode :   return ("isle") ;
        case GB_LOR_opcode :    return ("lor") ;
        case GB_LAND_opcode :   return ("land") ;
        case GB_LXOR_opcode :   return ("lxor") ;
        // case GB_BOR_opcode : ... bitwise ops
        // x | y, etc
        // case GB_PAIR_opcode :
        default :  ;
    }

    return ("") ;
}


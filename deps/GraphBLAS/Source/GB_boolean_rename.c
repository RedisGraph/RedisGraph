//------------------------------------------------------------------------------
// GB_boolean_rename: rename a boolean opcode
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Returns the equivalent opcode when an operator's x and y arguments are
// boolean.  15 of the 25 binary opcodes are redundant when applied to
// boolean inputs, leaving 10 unique binary opcodes z=f(x,y) when all three
// operands x,y,z are boolean.

// Another 4 boolean operators are not considered here since they share
// the same opcode:

// GrB_LOR  == GxB_LOR_BOOL     GB_LOR_binop_code
// GrB_LAND == GxB_LAND_BOOL    GB_LAND_binop_code
// GrB_LXOR == GxB_LXOR_BOOL    GB_LXOR_binop_code
// GrB_LXNOR == GxB_EQ_BOOL     GB_EQ_binop_code

// Those 6 names are in GraphBLAS but the pairs of names are equivalent.

// See discussion on Source/GB.h on boolean and integer division

#include "GB.h"
#include "GB_binop.h"

GB_Opcode GB_boolean_rename     // renamed opcode
(
    const GB_Opcode opcode      // binary opcode to rename
)
{

    switch (opcode)
    {

        // FIRST and DIV are the same for boolean:
        case GB_DIV_binop_code     :            // z = x / y
            return (GB_FIRST_binop_code) ;      // z = x

        // SECOND and RDIV are the same for boolean:
        case GB_RDIV_binop_code    :            // z = y / x
            return (GB_SECOND_binop_code) ;     // z = y

        // MIN, TIMES, and AND are the same for boolean:
        case GB_MIN_binop_code     :            // z = min(x,y)
        case GB_TIMES_binop_code   :            // z = x * y
            return (GB_LAND_binop_code) ;       // z = x && y

        // MAX, PLUS, and OR are the same for boolean:
        case GB_MAX_binop_code     :            // z = max(x,y)
        case GB_PLUS_binop_code    :            // z = x + y
            return (GB_LOR_binop_code) ;        // z = x || y

        // ISNE, NE, MINUS, RMINUS, and XOR are the same for boolean:
        case GB_MINUS_binop_code   :            // z = x - y
        case GB_RMINUS_binop_code  :            // z = y - x
        case GB_ISNE_binop_code    :            // z = (x != y)
        case GB_NE_binop_code      :            // z = (x != y)
            return (GB_LXOR_binop_code) ;       // z = (x != y)

        // ISEQ, EQ are the same for boolean:
        case GB_ISEQ_binop_code    :            // z = (x == y)
            return (GB_EQ_binop_code) ;

        // ISGT, GT are the same for boolean:
        case GB_ISGT_binop_code    :            // z = (x > y)
            return (GB_GT_binop_code) ;

        // ISLT, LT are the same for boolean:
        case GB_ISLT_binop_code    :            // z = (x < y)
            return (GB_LT_binop_code) ;

        // POW, ISGE, GE are the same for boolean:
        case GB_POW_binop_code     :            // z = (x to the y)
        case GB_ISGE_binop_code    :            // z = (x >= y)
            return (GB_GE_binop_code) ;

        // ISLE, LE are the same for boolean:
        case GB_ISLE_binop_code    :            // z = (x <= y)
            return (GB_LE_binop_code) ;

        // opcode not renamed
        default : 
            return (opcode) ;
    }
}


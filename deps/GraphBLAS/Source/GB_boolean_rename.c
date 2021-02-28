//------------------------------------------------------------------------------
// GB_boolean_rename: rename a boolean opcode
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns the equivalent opcode when an operator's x and y arguments are
// boolean.  15 of the 25 binary opcodes are redundant when applied to
// boolean inputs, leaving 10 unique binary opcodes z=f(x,y) when all three
// operands x,y,z are boolean.

// Another 3 boolean operators are not considered here since they share
// the same opcode:

// GrB_LOR  == GxB_LOR_BOOL,
// GrB_LAND == GxB_LAND_BOOL,
// GrB_LXOR == GxB_LXOR_BOOL

// Those 6 names are in GraphBLAS but the pairs of names are equivalent.

// See discussion on Source/GB.h on boolean and integer division

#include "GB.h"

GB_Opcode GB_boolean_rename     // renamed opcode
(
    const GB_Opcode opcode      // opcode to rename
)
{

    switch (opcode)
    {

        // FIRST and DIV are the same for boolean:
        case GB_DIV_opcode     :            // z = x / y
            return (GB_FIRST_opcode) ;      // z = x

        // SECOND and RDIV are the same for boolean:
        case GB_RDIV_opcode    :            // z = y / x
            return (GB_SECOND_opcode) ;     // z = y

        // MIN, TIMES, and AND are the same for boolean:
        case GB_MIN_opcode     :            // z = min(x,y)
        case GB_TIMES_opcode   :            // z = x * y
            return (GB_LAND_opcode) ;       // z = x && y

        // MAX, PLUS, and OR are the same for boolean:
        case GB_MAX_opcode     :            // z = max(x,y)
        case GB_PLUS_opcode    :            // z = x + y
            return (GB_LOR_opcode) ;        // z = x || y

        // ISNE, NE, MINUS, RMINUS, and XOR are the same for boolean:
        case GB_MINUS_opcode   :            // z = x - y
        case GB_RMINUS_opcode  :            // z = y - x
        case GB_ISNE_opcode    :            // z = (x != y)
        case GB_NE_opcode      :            // z = (x != y)
            return (GB_LXOR_opcode) ;       // z = (x != y)

        // ISEQ, EQ are the same for boolean:
        case GB_ISEQ_opcode    :            // z = (x == y)
            return (GB_EQ_opcode) ;

        // ISGT, GT are the same for boolean:
        case GB_ISGT_opcode    :            // z = (x > y)
            return (GB_GT_opcode) ;

        // ISLT, LT are the same for boolean:
        case GB_ISLT_opcode    :            // z = (x < y)
            return (GB_LT_opcode) ;

        // ISGE, GE are the same for boolean:
        case GB_ISGE_opcode    :            // z = (x >= y)
            return (GB_GE_opcode) ;

        // ISLE, LE are the same for boolean:
        case GB_ISLE_opcode    :            // z = (x <= y)
            return (GB_LE_opcode) ;

        // opcode not renamed
        default : 
            return (opcode) ;
    }
}


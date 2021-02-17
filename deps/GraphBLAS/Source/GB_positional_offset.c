//------------------------------------------------------------------------------
// GB_positional_offset: return the offset of a positional operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

int64_t GB_positional_offset        // return 0 or 1
(
    GB_Opcode opcode                // opcode of positional operator
)
{

    switch (opcode)
    {

        // these operators are offset by one
        case GB_POSITIONI1_opcode : // z = position_i1(A(i,j)) == i+1
        case GB_FIRSTI1_opcode    : // z = first_i1(A(i,j),y) == i+1
        case GB_SECONDI1_opcode   : // z = second_i1(x,A(i,j)) == i+1
        case GB_POSITIONJ1_opcode : // z = position_j1(A(i,j)) == j+1
        case GB_FIRSTJ1_opcode    : // z = first_j1(A(i,j),y) == j+1
        case GB_SECONDJ1_opcode   : // z = second_j1(x,A(i,j)) == j+1
            return (1) ;

        // all other operators have no offset
        default:
            return (0) ;
    }
}


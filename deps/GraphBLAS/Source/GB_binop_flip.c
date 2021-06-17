//------------------------------------------------------------------------------
// GB_binop_flip:  flip a binary multipy operator in a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Positional operators are flipped both in (first,second), but also (i,j).
// This function is only used for semirings, for matrix-matrix multiply.
// It is not used for GrB_apply or GrB_eWise*.

#include "GB.h"
#include "GB_binop.h"

GB_Opcode GB_binop_flip     // flipped opcode
(
    GB_Opcode opcode        // opcode to flip
)
{

    switch (opcode)
    {
        // swap FIRST and SECOND
        case GB_FIRST_opcode  : return (GB_SECOND_opcode) ;
        case GB_SECOND_opcode : return (GB_FIRST_opcode) ;

        // swap LT and GT
        case GB_GT_opcode     : return (GB_LT_opcode) ;
        case GB_LT_opcode     : return (GB_GT_opcode) ;

        // swap LE and GE
        case GB_GE_opcode     : return (GB_LE_opcode) ;
        case GB_LE_opcode     : return (GB_GE_opcode) ;

        // swap ISLT and ISGT
        case GB_ISGT_opcode   : return (GB_ISLT_opcode) ;
        case GB_ISLT_opcode   : return (GB_ISGT_opcode) ;

        // swap ISLE and ISGE
        case GB_ISGE_opcode   : return (GB_ISLE_opcode) ;
        case GB_ISLE_opcode   : return (GB_ISGE_opcode) ;

        // swap DIV and RDIV
        case GB_DIV_opcode    : return (GB_RDIV_opcode) ;
        case GB_RDIV_opcode   : return (GB_DIV_opcode) ;

        // swap MINUS and RMINUS
        case GB_MINUS_opcode  : return (GB_RMINUS_opcode) ;
        case GB_RMINUS_opcode : return (GB_MINUS_opcode) ;

        // swap FIRSTI and SECONDJ
        case GB_FIRSTI_opcode  : return (GB_SECONDJ_opcode) ;
        case GB_SECONDJ_opcode : return (GB_FIRSTI_opcode) ;

        // swap FIRSTI1 and SECONDJ1
        case GB_FIRSTI1_opcode  : return (GB_SECONDJ1_opcode) ;
        case GB_SECONDJ1_opcode : return (GB_FIRSTI1_opcode) ;

        // swap FIRSTJ and SECONDI
        case GB_FIRSTJ_opcode  : return (GB_SECONDI_opcode) ;
        case GB_SECONDI_opcode : return (GB_FIRSTJ_opcode) ;

        // swap FIRSTJ1 and SECONDI1
        case GB_FIRSTJ1_opcode  : return (GB_SECONDI1_opcode) ;
        case GB_SECONDI1_opcode : return (GB_FIRSTJ1_opcode) ;

        // these operators do not have flipped versions:
        // POW, BGET, BSET, BCLR, BSHIFT, ATAN2, FMOD, REMAINDER, COPYSIGN,
        // LDEXP, CMPLX, and user-defined operators.

        // these operators are commutative; they are their own flipped ops:
        // PLUS, TIMES, PAIR, ANY, ISEQ, ISNE, EQ, NE, MIN, MAX, LOR, LAND,
        // LXOR, LXNOR, HYPOT, BOR, BAND, BXOR, BXNOR.
        default:
            return (opcode) ;
    }
}


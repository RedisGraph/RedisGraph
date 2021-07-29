//------------------------------------------------------------------------------
// GB_flip_opcode:  flip a binary multiply operator in a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Positional operators are flipped both in (first,second), but also (i,j).
// This function is only used for semirings, for matrix-matrix multiply.
// It is not used for GrB_apply or GrB_eWise*.

#include "GB.h"
#include "GB_binop.h"

GB_Opcode GB_flip_opcode    // flipped opcode, or -1 on error
(
    GB_Opcode opcode,       // opcode to flip
    bool *handled           // true if opcode is handled by flipping the opcode
)
{

    (*handled) = true ;     // set below to false if the op is not handled

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

        // these operators are not commutative and do not have flipped ops:
        case GB_POW_opcode          :
        case GB_BGET_opcode         :
        case GB_BSET_opcode         :
        case GB_BCLR_opcode         :
        case GB_BSHIFT_opcode       :
        case GB_ATAN2_opcode        :
        case GB_FMOD_opcode         :
        case GB_REMAINDER_opcode    :
        case GB_COPYSIGN_opcode     :
        case GB_LDEXP_opcode        :
        case GB_CMPLX_opcode        :
        case GB_USER_opcode         :
            (*handled) = false ;
            return (opcode) ;

        // these operators are commutative; they are their own flipped ops:
        case GB_ANY_opcode          :
        case GB_PAIR_opcode         :
        case GB_MIN_opcode          :
        case GB_MAX_opcode          :
        case GB_PLUS_opcode         :
        case GB_TIMES_opcode        :
        case GB_ISEQ_opcode         :
        case GB_ISNE_opcode         :
        case GB_LOR_opcode          :
        case GB_LAND_opcode         :
        case GB_LXOR_opcode         :
        case GB_BOR_opcode          :
        case GB_BAND_opcode         :
        case GB_BXOR_opcode         :
        case GB_BXNOR_opcode        :
        case GB_EQ_opcode           :
        case GB_NE_opcode           :
        case GB_HYPOT_opcode        :
            return (opcode) ;

        default:
            // not a valid binary opcode
            (*handled) = false ;
            return (GB_BAD_opcode) ;
    }
}


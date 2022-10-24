//------------------------------------------------------------------------------
// GB_flip_binop_code:  flip a binary multiply operator in a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Positional operators are flipped both in (first,second), but also (i,j).
// This function is only used for semirings, for matrix-matrix multiply.
// It is not used for GrB_apply or GrB_eWise*.

#include "GB.h"
#include "GB_binop.h"

GB_Opcode GB_flip_binop_code    // flipped binary opcode, or -1 on error
(
    GB_Opcode opcode,       // binary opcode to flip
    bool *handled           // true if opcode is handled by flipping the opcode
)
{

    (*handled) = true ;     // set below to false if the op is not handled

    switch (opcode)
    {
        // swap FIRST and SECOND
        case GB_FIRST_binop_code  : return (GB_SECOND_binop_code) ;
        case GB_SECOND_binop_code : return (GB_FIRST_binop_code) ;

        // swap LT and GT
        case GB_GT_binop_code     : return (GB_LT_binop_code) ;
        case GB_LT_binop_code     : return (GB_GT_binop_code) ;

        // swap LE and GE
        case GB_GE_binop_code     : return (GB_LE_binop_code) ;
        case GB_LE_binop_code     : return (GB_GE_binop_code) ;

        // swap ISLT and ISGT
        case GB_ISGT_binop_code   : return (GB_ISLT_binop_code) ;
        case GB_ISLT_binop_code   : return (GB_ISGT_binop_code) ;

        // swap ISLE and ISGE
        case GB_ISGE_binop_code   : return (GB_ISLE_binop_code) ;
        case GB_ISLE_binop_code   : return (GB_ISGE_binop_code) ;

        // swap DIV and RDIV
        case GB_DIV_binop_code    : return (GB_RDIV_binop_code) ;
        case GB_RDIV_binop_code   : return (GB_DIV_binop_code) ;

        // swap MINUS and RMINUS
        case GB_MINUS_binop_code  : return (GB_RMINUS_binop_code) ;
        case GB_RMINUS_binop_code : return (GB_MINUS_binop_code) ;

        // swap FIRSTI and SECONDJ
        case GB_FIRSTI_binop_code  : return (GB_SECONDJ_binop_code) ;
        case GB_SECONDJ_binop_code : return (GB_FIRSTI_binop_code) ;

        // swap FIRSTI1 and SECONDJ1
        case GB_FIRSTI1_binop_code  : return (GB_SECONDJ1_binop_code) ;
        case GB_SECONDJ1_binop_code : return (GB_FIRSTI1_binop_code) ;

        // swap FIRSTJ and SECONDI
        case GB_FIRSTJ_binop_code  : return (GB_SECONDI_binop_code) ;
        case GB_SECONDI_binop_code : return (GB_FIRSTJ_binop_code) ;

        // swap FIRSTJ1 and SECONDI1
        case GB_FIRSTJ1_binop_code  : return (GB_SECONDI1_binop_code) ;
        case GB_SECONDI1_binop_code : return (GB_FIRSTJ1_binop_code) ;

        // these operators are not commutative and do not have flipped ops:
        case GB_POW_binop_code          :
        case GB_BGET_binop_code         :
        case GB_BSET_binop_code         :
        case GB_BCLR_binop_code         :
        case GB_BSHIFT_binop_code       :
        case GB_ATAN2_binop_code        :
        case GB_FMOD_binop_code         :
        case GB_REMAINDER_binop_code    :
        case GB_COPYSIGN_binop_code     :
        case GB_LDEXP_binop_code        :
        case GB_CMPLX_binop_code        :
        case GB_USER_binop_code         :
        default:
            (*handled) = false ;
            return (opcode) ;

        // these operators are commutative; they are their own flipped ops:
        case GB_ANY_binop_code          :
        case GB_PAIR_binop_code         :
        case GB_MIN_binop_code          :
        case GB_MAX_binop_code          :
        case GB_PLUS_binop_code         :
        case GB_TIMES_binop_code        :
        case GB_ISEQ_binop_code         :
        case GB_ISNE_binop_code         :
        case GB_LOR_binop_code          :
        case GB_LAND_binop_code         :
        case GB_LXOR_binop_code         :
        case GB_BOR_binop_code          :
        case GB_BAND_binop_code         :
        case GB_BXOR_binop_code         :
        case GB_BXNOR_binop_code        :
        case GB_EQ_binop_code           :
        case GB_NE_binop_code           :
        case GB_HYPOT_binop_code        :
            return (opcode) ;
    }
}


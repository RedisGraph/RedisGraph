//------------------------------------------------------------------------------
// GB_flip_op:  flip a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Positional operators are flipped both in (first,second), but also (i,j).
// This function is only used for ewise operators.

#include "GB.h"
#include "GB_binop.h"

GrB_BinaryOp GB_flip_op     // flip a binary operator
(
    GrB_BinaryOp op,        // binary operator to flip
    bool *handled           // true if operator is handled
)
{

    (*handled) = true ;     // set below to false if the op is not handled

    GB_Type_code xcode = op->xtype->code ;

    switch (op->opcode)
    {

        //----------------------------------------------------------------------
        // swap FIRST and SECOND
        //----------------------------------------------------------------------

        case GB_FIRST_opcode  :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_SECOND_BOOL) ;
                case GB_INT8_code   : return (GrB_SECOND_INT8) ;
                case GB_INT16_code  : return (GrB_SECOND_INT16) ;
                case GB_INT32_code  : return (GrB_SECOND_INT32) ;
                case GB_INT64_code  : return (GrB_SECOND_INT64) ;
                case GB_UINT8_code  : return (GrB_SECOND_UINT8) ;
                case GB_UINT16_code : return (GrB_SECOND_UINT16) ;
                case GB_UINT32_code : return (GrB_SECOND_UINT32) ;
                case GB_UINT64_code : return (GrB_SECOND_UINT64) ;
                case GB_FP32_code   : return (GrB_SECOND_FP32) ;
                case GB_FP64_code   : return (GrB_SECOND_FP64) ;
                case GB_FC32_code   : return (GxB_SECOND_FC32) ;
                case GB_FC64_code   : return (GxB_SECOND_FC64) ;
                default: ;
            }
            break ;

        case GB_SECOND_opcode :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_FIRST_BOOL) ;
                case GB_INT8_code   : return (GrB_FIRST_INT8) ;
                case GB_INT16_code  : return (GrB_FIRST_INT16) ;
                case GB_INT32_code  : return (GrB_FIRST_INT32) ;
                case GB_INT64_code  : return (GrB_FIRST_INT64) ;
                case GB_UINT8_code  : return (GrB_FIRST_UINT8) ;
                case GB_UINT16_code : return (GrB_FIRST_UINT16) ;
                case GB_UINT32_code : return (GrB_FIRST_UINT32) ;
                case GB_UINT64_code : return (GrB_FIRST_UINT64) ;
                case GB_FP32_code   : return (GrB_FIRST_FP32) ;
                case GB_FP64_code   : return (GrB_FIRST_FP64) ;
                case GB_FC32_code   : return (GxB_FIRST_FC32) ;
                case GB_FC64_code   : return (GxB_FIRST_FC64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // swap LT and GT
        //----------------------------------------------------------------------

        case GB_GT_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_LT_BOOL) ;
                case GB_INT8_code   : return (GrB_LT_INT8) ;
                case GB_INT16_code  : return (GrB_LT_INT16) ;
                case GB_INT32_code  : return (GrB_LT_INT32) ;
                case GB_INT64_code  : return (GrB_LT_INT64) ;
                case GB_UINT8_code  : return (GrB_LT_UINT8) ;
                case GB_UINT16_code : return (GrB_LT_UINT16) ;
                case GB_UINT32_code : return (GrB_LT_UINT32) ;
                case GB_UINT64_code : return (GrB_LT_UINT64) ;
                case GB_FP32_code   : return (GrB_LT_FP32) ;
                case GB_FP64_code   : return (GrB_LT_FP64) ;
                default: ;
            }
            break ;

        case GB_LT_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_GT_BOOL) ;
                case GB_INT8_code   : return (GrB_GT_INT8) ;
                case GB_INT16_code  : return (GrB_GT_INT16) ;
                case GB_INT32_code  : return (GrB_GT_INT32) ;
                case GB_INT64_code  : return (GrB_GT_INT64) ;
                case GB_UINT8_code  : return (GrB_GT_UINT8) ;
                case GB_UINT16_code : return (GrB_GT_UINT16) ;
                case GB_UINT32_code : return (GrB_GT_UINT32) ;
                case GB_UINT64_code : return (GrB_GT_UINT64) ;
                case GB_FP32_code   : return (GrB_GT_FP32) ;
                case GB_FP64_code   : return (GrB_GT_FP64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // swap LE and GE
        //----------------------------------------------------------------------

        case GB_GE_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_LE_BOOL) ;
                case GB_INT8_code   : return (GrB_LE_INT8) ;
                case GB_INT16_code  : return (GrB_LE_INT16) ;
                case GB_INT32_code  : return (GrB_LE_INT32) ;
                case GB_INT64_code  : return (GrB_LE_INT64) ;
                case GB_UINT8_code  : return (GrB_LE_UINT8) ;
                case GB_UINT16_code : return (GrB_LE_UINT16) ;
                case GB_UINT32_code : return (GrB_LE_UINT32) ;
                case GB_UINT64_code : return (GrB_LE_UINT64) ;
                case GB_FP32_code   : return (GrB_LE_FP32) ;
                case GB_FP64_code   : return (GrB_LE_FP64) ;
                default: ;
            }
            break ;

        case GB_LE_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_GE_BOOL) ;
                case GB_INT8_code   : return (GrB_GE_INT8) ;
                case GB_INT16_code  : return (GrB_GE_INT16) ;
                case GB_INT32_code  : return (GrB_GE_INT32) ;
                case GB_INT64_code  : return (GrB_GE_INT64) ;
                case GB_UINT8_code  : return (GrB_GE_UINT8) ;
                case GB_UINT16_code : return (GrB_GE_UINT16) ;
                case GB_UINT32_code : return (GrB_GE_UINT32) ;
                case GB_UINT64_code : return (GrB_GE_UINT64) ;
                case GB_FP32_code   : return (GrB_GE_FP32) ;
                case GB_FP64_code   : return (GrB_GE_FP64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // swap ISLT and ISGT
        //----------------------------------------------------------------------

        case GB_ISGT_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GxB_ISLT_BOOL) ;
                case GB_INT8_code   : return (GxB_ISLT_INT8) ;
                case GB_INT16_code  : return (GxB_ISLT_INT16) ;
                case GB_INT32_code  : return (GxB_ISLT_INT32) ;
                case GB_INT64_code  : return (GxB_ISLT_INT64) ;
                case GB_UINT8_code  : return (GxB_ISLT_UINT8) ;
                case GB_UINT16_code : return (GxB_ISLT_UINT16) ;
                case GB_UINT32_code : return (GxB_ISLT_UINT32) ;
                case GB_UINT64_code : return (GxB_ISLT_UINT64) ;
                case GB_FP32_code   : return (GxB_ISLT_FP32) ;
                case GB_FP64_code   : return (GxB_ISLT_FP64) ;
                default: ;
            }
            break ;

        case GB_ISLT_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GxB_ISGT_BOOL) ;
                case GB_INT8_code   : return (GxB_ISGT_INT8) ;
                case GB_INT16_code  : return (GxB_ISGT_INT16) ;
                case GB_INT32_code  : return (GxB_ISGT_INT32) ;
                case GB_INT64_code  : return (GxB_ISGT_INT64) ;
                case GB_UINT8_code  : return (GxB_ISGT_UINT8) ;
                case GB_UINT16_code : return (GxB_ISGT_UINT16) ;
                case GB_UINT32_code : return (GxB_ISGT_UINT32) ;
                case GB_UINT64_code : return (GxB_ISGT_UINT64) ;
                case GB_FP32_code   : return (GxB_ISGT_FP32) ;
                case GB_FP64_code   : return (GxB_ISGT_FP64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // swap ISLE and ISGE
        //----------------------------------------------------------------------

        case GB_ISGE_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GxB_ISLE_BOOL) ;
                case GB_INT8_code   : return (GxB_ISLE_INT8) ;
                case GB_INT16_code  : return (GxB_ISLE_INT16) ;
                case GB_INT32_code  : return (GxB_ISLE_INT32) ;
                case GB_INT64_code  : return (GxB_ISLE_INT64) ;
                case GB_UINT8_code  : return (GxB_ISLE_UINT8) ;
                case GB_UINT16_code : return (GxB_ISLE_UINT16) ;
                case GB_UINT32_code : return (GxB_ISLE_UINT32) ;
                case GB_UINT64_code : return (GxB_ISLE_UINT64) ;
                case GB_FP32_code   : return (GxB_ISLE_FP32) ;
                case GB_FP64_code   : return (GxB_ISLE_FP64) ;
                default: ;
            }
            break ;

        case GB_ISLE_opcode     :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GxB_ISGE_BOOL) ;
                case GB_INT8_code   : return (GxB_ISGE_INT8) ;
                case GB_INT16_code  : return (GxB_ISGE_INT16) ;
                case GB_INT32_code  : return (GxB_ISGE_INT32) ;
                case GB_INT64_code  : return (GxB_ISGE_INT64) ;
                case GB_UINT8_code  : return (GxB_ISGE_UINT8) ;
                case GB_UINT16_code : return (GxB_ISGE_UINT16) ;
                case GB_UINT32_code : return (GxB_ISGE_UINT32) ;
                case GB_UINT64_code : return (GxB_ISGE_UINT64) ;
                case GB_FP32_code   : return (GxB_ISGE_FP32) ;
                case GB_FP64_code   : return (GxB_ISGE_FP64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // swap DIV and RDIV
        //----------------------------------------------------------------------

        case GB_DIV_opcode  :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GxB_RDIV_BOOL) ;
                case GB_INT8_code   : return (GxB_RDIV_INT8) ;
                case GB_INT16_code  : return (GxB_RDIV_INT16) ;
                case GB_INT32_code  : return (GxB_RDIV_INT32) ;
                case GB_INT64_code  : return (GxB_RDIV_INT64) ;
                case GB_UINT8_code  : return (GxB_RDIV_UINT8) ;
                case GB_UINT16_code : return (GxB_RDIV_UINT16) ;
                case GB_UINT32_code : return (GxB_RDIV_UINT32) ;
                case GB_UINT64_code : return (GxB_RDIV_UINT64) ;
                case GB_FP32_code   : return (GxB_RDIV_FP32) ;
                case GB_FP64_code   : return (GxB_RDIV_FP64) ;
                case GB_FC32_code   : return (GxB_RDIV_FC32) ;
                case GB_FC64_code   : return (GxB_RDIV_FC64) ;
                default: ;
            }
            break ;

        case GB_RDIV_opcode :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_DIV_BOOL) ;
                case GB_INT8_code   : return (GrB_DIV_INT8) ;
                case GB_INT16_code  : return (GrB_DIV_INT16) ;
                case GB_INT32_code  : return (GrB_DIV_INT32) ;
                case GB_INT64_code  : return (GrB_DIV_INT64) ;
                case GB_UINT8_code  : return (GrB_DIV_UINT8) ;
                case GB_UINT16_code : return (GrB_DIV_UINT16) ;
                case GB_UINT32_code : return (GrB_DIV_UINT32) ;
                case GB_UINT64_code : return (GrB_DIV_UINT64) ;
                case GB_FP32_code   : return (GrB_DIV_FP32) ;
                case GB_FP64_code   : return (GrB_DIV_FP64) ;
                case GB_FC32_code   : return (GxB_DIV_FC32) ;
                case GB_FC64_code   : return (GxB_DIV_FC64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // swap MINUS and RMINUS
        //----------------------------------------------------------------------

        case GB_MINUS_opcode  :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GxB_RMINUS_BOOL) ;
                case GB_INT8_code   : return (GxB_RMINUS_INT8) ;
                case GB_INT16_code  : return (GxB_RMINUS_INT16) ;
                case GB_INT32_code  : return (GxB_RMINUS_INT32) ;
                case GB_INT64_code  : return (GxB_RMINUS_INT64) ;
                case GB_UINT8_code  : return (GxB_RMINUS_UINT8) ;
                case GB_UINT16_code : return (GxB_RMINUS_UINT16) ;
                case GB_UINT32_code : return (GxB_RMINUS_UINT32) ;
                case GB_UINT64_code : return (GxB_RMINUS_UINT64) ;
                case GB_FP32_code   : return (GxB_RMINUS_FP32) ;
                case GB_FP64_code   : return (GxB_RMINUS_FP64) ;
                case GB_FC32_code   : return (GxB_RMINUS_FC32) ;
                case GB_FC64_code   : return (GxB_RMINUS_FC64) ;
                default: ;
            }
            break ;

        case GB_RMINUS_opcode :

            switch (xcode)
            {
                case GB_BOOL_code   : return (GrB_MINUS_BOOL) ;
                case GB_INT8_code   : return (GrB_MINUS_INT8) ;
                case GB_INT16_code  : return (GrB_MINUS_INT16) ;
                case GB_INT32_code  : return (GrB_MINUS_INT32) ;
                case GB_INT64_code  : return (GrB_MINUS_INT64) ;
                case GB_UINT8_code  : return (GrB_MINUS_UINT8) ;
                case GB_UINT16_code : return (GrB_MINUS_UINT16) ;
                case GB_UINT32_code : return (GrB_MINUS_UINT32) ;
                case GB_UINT64_code : return (GrB_MINUS_UINT64) ;
                case GB_FP32_code   : return (GrB_MINUS_FP32) ;
                case GB_FP64_code   : return (GrB_MINUS_FP64) ;
                case GB_FC32_code   : return (GxB_MINUS_FC32) ;
                case GB_FC64_code   : return (GxB_MINUS_FC64) ;
                default: ;
            }
            break ;

        //----------------------------------------------------------------------
        // positional operators do not need to be flipped for ewise methods
        //----------------------------------------------------------------------

        case GB_FIRSTI_opcode       :
        case GB_FIRSTJ_opcode       :
        case GB_FIRSTI1_opcode      :
        case GB_FIRSTJ1_opcode      :
        case GB_SECONDI_opcode      :
        case GB_SECONDJ_opcode      :
        case GB_SECONDI1_opcode     :
        case GB_SECONDJ1_opcode     :
            return (op) ;

        //----------------------------------------------------------------------
        // these operators are not commutative and do not have flipped ops:
        //----------------------------------------------------------------------

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
            return (op) ;

        //----------------------------------------------------------------------
        // these operators are commutative; they are their own flipped ops:
        //----------------------------------------------------------------------

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
            return (op) ;

        default: ;
    }

    //--------------------------------------------------------------------------
    // invalid operator
    //--------------------------------------------------------------------------

    (*handled) = false ;
    return (NULL) ;
}


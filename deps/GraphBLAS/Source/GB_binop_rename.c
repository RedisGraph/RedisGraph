//------------------------------------------------------------------------------
// GB_binop_rename: rename a bound binary operator to its unary op equivalent
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A binary operator can be applied to a matrix with GrB_apply, but some of
// them can be remapped into equivalent unary operators.

#include "GB.h"
#include "GB_binop.h"

void GB_binop_rename            // rename a bound binary op
(
    GrB_UnaryOp *op1,           // set to new unary op, if op2 is renamed
    GrB_BinaryOp *op2,          // set to NULL if op2 is renamed
    bool binop_bind1st
)
{

    if ((*op2) != NULL)
    {

        //----------------------------------------------------------------------
        // get the opcode and type, and rename if boolean
        //----------------------------------------------------------------------

        GB_Opcode opcode = (*op2)->opcode ;
        GB_Type_code xcode = (*op2)->xtype->code ;
        if (xcode == GB_BOOL_code)
        { 
            // for boolean: DIV becomes FIRST, RDIV becomes SECOND
            opcode = GB_boolean_rename (opcode) ;
        }

        //----------------------------------------------------------------------
        // rename the binary op to an equivalent unary op, if possible
        //----------------------------------------------------------------------

        if ((opcode == GB_FIRST_opcode  && !binop_bind1st) ||
            (opcode == GB_SECOND_opcode &&  binop_bind1st))
        { 

            //------------------------------------------------------------------
            // first(A,scalar) and second(scalar,A) become identity(A)
            //------------------------------------------------------------------

            switch (xcode)
            {
                case GB_BOOL_code    : (*op1) = GrB_IDENTITY_BOOL   ; break ;
                case GB_INT8_code    : (*op1) = GrB_IDENTITY_INT8   ; break ;
                case GB_INT16_code   : (*op1) = GrB_IDENTITY_INT16  ; break ;
                case GB_INT32_code   : (*op1) = GrB_IDENTITY_INT32  ; break ;
                case GB_INT64_code   : (*op1) = GrB_IDENTITY_INT64  ; break ;
                case GB_UINT8_code   : (*op1) = GrB_IDENTITY_UINT8  ; break ;
                case GB_UINT16_code  : (*op1) = GrB_IDENTITY_UINT16 ; break ;
                case GB_UINT32_code  : (*op1) = GrB_IDENTITY_UINT32 ; break ;
                case GB_UINT64_code  : (*op1) = GrB_IDENTITY_UINT64 ; break ;
                case GB_FP32_code    : (*op1) = GrB_IDENTITY_FP32   ; break ;
                case GB_FP64_code    : (*op1) = GrB_IDENTITY_FP64   ; break ;
                case GB_FC32_code    : (*op1) = GxB_IDENTITY_FC32   ; break ;
                case GB_FC64_code    : (*op1) = GxB_IDENTITY_FC64   ; break ;
                default:;
            }

        }
        else if (opcode == GB_PAIR_opcode)
        { 

            //------------------------------------------------------------------
            // pair(A,scalar) and pair(scalar,A) become one(A)
            //------------------------------------------------------------------

            (*op1) = GB_unop_one (xcode) ;
        }

        if ((*op1) != NULL)
        { 
            (*op2) = NULL ;
        }
    }
}


//------------------------------------------------------------------------------
// GB_iso_unop_code: determine if C = A, C = op1(A), or C = op2(A) is iso
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

// C = A, C = op1(A), C = op2(A,scalar), or C = op2(scalar,A) is being
// computed, with A optionally transposed.  Determine if C is iso.

GB_iso_code GB_iso_unop_code
(
    GrB_Matrix A,           // input matrix
    GrB_UnaryOp op1,        // unary operator, if present
    GrB_BinaryOp op2,       // binary operator, if present
    bool binop_bind1st      // if true, C = op2(x,A), otherwise C = op2(A,y)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // get the opcode
    //--------------------------------------------------------------------------

    GB_Opcode opcode = GB_NOP_opcode ;
    if (op1 != NULL) opcode = op1->opcode ;
    if (op2 != NULL) opcode = op2->opcode ;

    //--------------------------------------------------------------------------
    // positional ops never result in an iso matrix
    //--------------------------------------------------------------------------

    if (GB_OPCODE_IS_POSITIONAL (opcode))
    { 
        // this is the only case where C is non-iso even if A is iso
        return (GB_NON_ISO) ;
    }

    //--------------------------------------------------------------------------
    // C = op1 (A) or pair (...)
    //--------------------------------------------------------------------------

    if ((opcode == GB_ONE_opcode) || (opcode == GB_PAIR_opcode))
    { 
        // if op1 is ONE or op2 is PAIR, then C is iso, with a value
        // equal to 1
        return (GB_ISO_1) ;
    }

    //--------------------------------------------------------------------------
    // C = op2 (scalar,A) or op2 (A,scalar)
    //--------------------------------------------------------------------------

    if ((opcode == GB_ANY_opcode) ||                        // C = any(...)
        (opcode == GB_FIRST_opcode  &&  binop_bind1st) ||   // C = first(x,A)
        (opcode == GB_SECOND_opcode && !binop_bind1st))     // C = second(A,y)
    { 
        // if op2 is FIRST and binop_bind1st is true, or if op2 is SECOND and
        // binop_bind1st is false, or if op2 is ANY, then C is iso, with a
        // value equal to the scalar
        return (GB_ISO_S) ;
    }

    //--------------------------------------------------------------------------
    // C is iso if A is iso
    //--------------------------------------------------------------------------

    if (A->iso)
    {
        if (opcode == GB_NOP_opcode || opcode == GB_IDENTITY_opcode)
        { 
            // C = (ctype) A
            return (GB_ISO_A) ;
        }
        else if (op1 != NULL)
        { 
            // C = op1 (A)
            return (GB_ISO_OP1_A) ;
        }
        else if (binop_bind1st)
        { 
            // C = op2 (scalar, A)
            return (GB_ISO_OP2_SA) ;
        }
        else
        { 
            // C = op2 (A, scalar)
            return (GB_ISO_OP2_AS) ;
        }
    }

    //--------------------------------------------------------------------------
    // otherwise, C is not iso
    //--------------------------------------------------------------------------

    return (GB_NON_ISO) ;
}


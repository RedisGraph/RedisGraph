//------------------------------------------------------------------------------
// GB_binop_builtin:  determine if a binary operator is built-in
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Determine if A*B uses a built-in semiring, and if so, determine the
// opcodes and type codes of the semiring.

// If the op is NULL, then it is the implicit GrB_SECOND_[A_type] operator.
// This is a built-in operator for built-in types.  This feature is only used
// by GB_Matrix_wait.

#include "GB.h"
#include "GB_binop.h"
#include "GB_unused.h"

bool GB_binop_builtin               // true if binary operator is builtin
(
    // inputs:
    const GrB_Type A_type,
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Type B_type,
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_BinaryOp op,          // binary operator; may be NULL
    const bool flipxy,              // true if z=op(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *opcode,              // opcode for the binary operator
    GB_Type_code *xcode,            // type code for x input
    GB_Type_code *ycode,            // type code for y input
    GB_Type_code *zcode             // type code for z output
)
{

    //--------------------------------------------------------------------------
    // check if the operator is builtin, with no typecasting
    //--------------------------------------------------------------------------

    GrB_Type op_xtype, op_ytype, op_ztype ;
    if (op == NULL)
    { 
        // implicit GB_SECOND_[TYPE] operator
        ASSERT (A_type == B_type) ;
        (*opcode) = GB_SECOND_opcode ;
        op_xtype = A_type ;
        op_ytype = A_type ;
        op_ztype = A_type ;
    }
    else
    { 
        (*opcode) = op->opcode ;
        op_xtype = op->xtype ;
        op_ytype = op->ytype ;
        op_ztype = op->ztype ;
    }

    if (*opcode >= GB_USER_opcode)
    { 
        // the binary operator is user-defined
        return (false) ;
    }

    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (*opcode) ;

    // check if A matches the input to the operator
    if (!A_is_pattern && !op_is_positional)
    {
        if ((A_type != (flipxy ? op_ytype : op_xtype)) ||
            (A_type->code >= GB_UDT_code))
        { 
            // A is a user-defined type, or its type does not match the input
            // to the operator
            return (false) ;
        }
    }

    // check if B matches the input to the operator
    if (!B_is_pattern && !op_is_positional)
    {
        if ((B_type != (flipxy ? op_xtype : op_ytype)) ||
            (B_type->code >= GB_UDT_code))
        { 
            // B is a user-defined type, or its type does not match the input
            // to the operator
            return (false) ;
        }
    }

    //--------------------------------------------------------------------------
    // rename redundant boolean operators
    //--------------------------------------------------------------------------

    (*xcode) = op_xtype->code ;
    (*ycode) = op_ytype->code ;
    (*zcode) = op_ztype->code ;

    ASSERT ((*xcode) < GB_UDT_code) ;
    ASSERT ((*ycode) < GB_UDT_code) ;
    ASSERT ((*zcode) < GB_UDT_code) ;

    if ((*xcode) == GB_BOOL_code)
    { 
        // z = op(x,y) where both x and y are boolean.
        // DIV becomes FIRST
        // RDIV becomes SECOND
        // MIN and TIMES become LAND
        // MAX and PLUS become LOR
        // NE, ISNE, RMINUS, and MINUS become LXOR
        // ISEQ becomes EQ
        // ISGT becomes GT
        // ISLT becomes LT
        // ISGE and POW become GE
        // ISLE becomes LE
        (*opcode) = GB_boolean_rename (*opcode) ;
    }

    //--------------------------------------------------------------------------
    // handle the flipxy (for a semiring only)
    //--------------------------------------------------------------------------

    // If flipxy is true, the matrices A and B have been flipped (A passed as B
    // and B passed as A), so pass A as the 2nd argument to the operator, and B
    // as the first.  This can also be done by flipping operator opcodes
    // instead of flipping the A and B inputs to the operator, thus simplifying
    // the workers.  The z=x-y and z=x/y operators are flipped using the GxB_*
    // functions rminus (z=y-x)and rdiv (z=y/x).

    if (flipxy)
    { 
        // All built-in semirings use either commutative multiplicative
        // operators (PLUS, TIMES, ANY, ...), or operators that have flipped
        // versions (DIV vs RDIV, ...).
        (*opcode) = GB_binop_flip (*opcode) ;
    }

    return (true) ;
}


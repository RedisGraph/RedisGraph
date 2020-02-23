//------------------------------------------------------------------------------
// GB_binop_builtin:  determine if a binary operator is built-in
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Determine if A*B uses a built-in semiring, and if so, determine the
// opcodes and type codes of the semiring.

// If the op is NULL, then it is the implicit GrB_SECOND_[A_type] operator.
// This is a built-in operator for built-in types.  This feature is only used
// by GB_wait.

#include "GB.h"

#ifndef GBCOMPACT

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
    GB_Type_code *xycode,           // type code for x and y inputs
    GB_Type_code *zcode             // type code for z output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // A and B may be aliased

    //--------------------------------------------------------------------------
    // check if the operator is builtin, with no typecasting
    //--------------------------------------------------------------------------

    GrB_Type op_xtype, op_ytype, op_ztype ;
    if (op == NULL)
    { 
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

    // This function requires A and B to have the same built-in type, and they
    // must match the types x,y for the binary operator.  If this condition
    // doesn't hold, punt to the generic function.
    if (!A_is_pattern)
    {
        if ((A_type != (flipxy ? op_ytype : op_xtype)) ||
            (A_type->code >= GB_UDT_code))
        { 
            // A is a user-defined type, or its type does not match the input
            // to the operator
            return (false) ;
        }
    }

    if (!B_is_pattern)
    {
        if ((B_type != (flipxy ? op_xtype : op_ytype)) ||
            (B_type->code >= GB_UDT_code))
        { 
            // B is a user-defined type, or its type does not match the input
            // to the operator
            return (false) ;
        }
    }

    if (!A_is_pattern && !B_is_pattern)
    {
        if (A_type != B_type)
        { 
            // the types of A and B must match
            return (false) ;
        }
    }

    if (*opcode >= GB_USER_opcode)
    { 
        // the binary operator is user-defined
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // rename redundant boolean operators
    //--------------------------------------------------------------------------

    (*xycode) = op_xtype->code ;
    (*zcode)  = op_ztype->code ;

    ASSERT ((*xycode) <= GB_UDT_code) ;
    ASSERT ((*zcode)  <= GB_UDT_code) ;

    if ((*xycode) == GB_BOOL_code)
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
        // ISGE becomes GE
        // ISLE becomes LE
        (*opcode) = GB_boolean_rename (*opcode) ;
    }

    // built-in binary operators always have this property.
    ASSERT ((*zcode) == GB_BOOL_code || (*zcode) == (*xycode)) ;

    //--------------------------------------------------------------------------
    // handle the flipxy
    //--------------------------------------------------------------------------

    // If flipxy is true, the matrices A and B have been flipped (A passed as B
    // and B passed as A), so pass A as the 2nd argument to the operator, and B
    // as the first.  This can also be done by flipping operator opcodes
    // instead of flipping the A and B inputs to the operator, thus simplifying
    // the workers.  The z=x-y and z=x/y operators are flipped using the GxB_*
    // functions rminus (z=y-x)and rdiv (z=y/x).

    if (flipxy)
    {
        switch (*opcode)
        {
            // swap FIRST and SECOND
            case GB_FIRST_opcode  : (*opcode) = GB_SECOND_opcode ; break ;
            case GB_SECOND_opcode : (*opcode) = GB_FIRST_opcode ;  break ;

            // swap LT and GT
            case GB_GT_opcode     : (*opcode) = GB_LT_opcode ;     break ;
            case GB_LT_opcode     : (*opcode) = GB_GT_opcode ;     break ;

            // swap LE and GE
            case GB_GE_opcode     : (*opcode) = GB_LE_opcode ;     break ;
            case GB_LE_opcode     : (*opcode) = GB_GE_opcode ;     break ;

            // swap ISLT and ISGT
            case GB_ISGT_opcode   : (*opcode) = GB_ISLT_opcode ;   break ;
            case GB_ISLT_opcode   : (*opcode) = GB_ISGT_opcode ;   break ;

            // swap ISLE and ISGE
            case GB_ISGE_opcode   : (*opcode) = GB_ISLE_opcode ;   break ;
            case GB_ISLE_opcode   : (*opcode) = GB_ISGE_opcode ;   break ;

            // swap DIV and RDIV
            case GB_DIV_opcode    : (*opcode) = GB_RDIV_opcode ;   break ;
            case GB_RDIV_opcode   : (*opcode) = GB_DIV_opcode ;    break ;

            // swap MINUS and RMINUS
            case GB_MINUS_opcode  : (*opcode) = GB_RMINUS_opcode ; break ;
            case GB_RMINUS_opcode : (*opcode) = GB_MINUS_opcode ;  break ;

            default: ;
        }
    }

    return (true) ;
}

#endif

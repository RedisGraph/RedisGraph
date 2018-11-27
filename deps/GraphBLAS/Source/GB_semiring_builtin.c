//------------------------------------------------------------------------------
// GB_semiring_builtin:  determine if semiring is built-in
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Determine if A*B uses a built-in semiring, and if so, determine the
// opcodes and type codes of the semiring.

#include "GB.h"

#ifndef GBCOMPACT

bool GB_semiring_builtin            // true if semiring is builtin
(
    // inputs:
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // true if z=fmult(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *mult_opcode,         // multiply opcode
    GB_Opcode *add_opcode,          // add opcode
    GB_Type_code *xycode,           // type code for x and y inputs
    GB_Type_code *zcode             // type code for z output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK (A, B)) ;

    GrB_BinaryOp add  = semiring->add->op ;     // add operator
    GrB_BinaryOp mult = semiring->multiply ;    // multiply operator

    (*add_opcode)  = add->opcode ;       // add opcode
    (*mult_opcode) = mult->opcode ;      // multiply opcode

    // add is a monoid
    ASSERT (add->xtype == add->ztype && add->ytype == add->ztype) ;

    // in a semiring, the ztypes of add and mult are always the same:
    ASSERT (add->ztype == mult->ztype) ;

    // The conditions above are true for any semiring and any A and B, whether
    // or not this function handles the semiring as hard-coded.  Now return for
    // cases this function does not handle.

    // This function requires A and B to have the same built-in type, and they
    // must match the types x,y for fmult.  If this condition doesn't hold,
    // punt to the generic C=A*B:
    if ((A->type != (flipxy ? mult->ytype : mult->xtype)) ||
        (B->type != (flipxy ? mult->xtype : mult->ytype)) ||
        (A->type != B->type) || (A->type->code >= GB_UCT_code) ||
        (*add_opcode >= GB_USER_C_opcode) || (*mult_opcode >= GB_USER_C_opcode))
    { 
        return (false) ;
    }

    // this condition is true for all built-in operators, but not required for
    // user-defined operators
    ASSERT (mult->xtype == mult->ytype) ;

    // all the inputs to mult(x,y) are now the same, with no casting
    ASSERT (A->type == B->type) ;
    ASSERT (A->type == mult->xtype) ;
    ASSERT (A->type == mult->ytype) ;

    //--------------------------------------------------------------------------
    // rename redundant boolean multiply operators
    //--------------------------------------------------------------------------

    (*xycode) = mult->xtype->code ;
    (*zcode)  = mult->ztype->code ;

    ASSERT ((*xycode) <= GB_UDT_code) ;
    ASSERT ((*zcode)  <= GB_UDT_code) ;

    if ((*xycode) == GB_BOOL_code)
    { 
        // z = mult(x,y) where both x and y are boolean.
        // DIV becomes FIRST
        // MIN and TIMES become LAND
        // MAX and PLUS become LOR
        // NE, ISNE, and MINUS become LXOR
        // ISEQ becomes EQ
        // ISGT becomes GT
        // ISLT becomes LT
        // ISGE becomes GE
        // ISLE becomes LE
        (*mult_opcode) = GB_boolean_rename (*mult_opcode) ;
    }

    if ((*zcode) == GB_BOOL_code)
    { 
        // Only the LAND, LOR, LXOR, and EQ monoids remain if z is
        // boolean.  MIN, MAX, PLUS, and TIMES are renamed.
        (*add_opcode) = GB_boolean_rename (*add_opcode) ;
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
    // the workers.  Only z=x-y and z=x/y are left unflipped; these are
    // computed using a ternary in the MINUS and DIV workers instead.

    if (flipxy)
    {
        switch (*mult_opcode)
        {
            // swap FIRST and SECOND
            case GB_FIRST_opcode  : (*mult_opcode) = GB_SECOND_opcode ; break ;
            case GB_SECOND_opcode : (*mult_opcode) = GB_FIRST_opcode ;  break ;

            // swap LT and GT
            case GB_GT_opcode     : (*mult_opcode) = GB_LT_opcode ;     break ;
            case GB_LT_opcode     : (*mult_opcode) = GB_GT_opcode ;     break ;

            // swap LE and GE
            case GB_GE_opcode     : (*mult_opcode) = GB_LE_opcode ;     break ;
            case GB_LE_opcode     : (*mult_opcode) = GB_GE_opcode ;     break ;

            // swap ISLT and ISGT
            case GB_ISGT_opcode   : (*mult_opcode) = GB_ISLT_opcode ;   break ;
            case GB_ISLT_opcode   : (*mult_opcode) = GB_ISGT_opcode ;   break ;

            // swap ISLE and ISGE
            case GB_ISGE_opcode   : (*mult_opcode) = GB_ISLE_opcode ;   break ;
            case GB_ISLE_opcode   : (*mult_opcode) = GB_ISGE_opcode ;   break ;

            default: ;
        }
    }

    return (true) ;
}

#endif

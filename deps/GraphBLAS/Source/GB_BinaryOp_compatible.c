//------------------------------------------------------------------------------
// GB_BinaryOp_compatible: check binary operator for type compatibility
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check type compatibilty for C = op (A,B).  With typecasting: A is cast to
// op->xtype, B is cast to op->ytype, the operator is computed, and then the
// result of op->ztype is cast to C->type.

#include "GB.h"

GrB_Info GB_BinaryOp_compatible     // check for domain mismatch
(
    const GrB_BinaryOp op,          // binary operator to check
    const GrB_Type ctype,           // C must be compatible with op->ztype
    const GrB_Type atype,           // A must be compatible with op->xtype
    const GrB_Type btype,           // B must be compatible with op->ytype
    const GB_Type_code bcode,       // B may not have a type, just a code
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (op != NULL) ;
    // ctype and btype may be NULL, but atype is never NULL
    ASSERT (atype != NULL) ;
    ASSERT (bcode <= GB_UDT_code) ;

    //--------------------------------------------------------------------------
    // first input A is cast into the type of op->xtype
    //--------------------------------------------------------------------------

    if (op->opcode == GB_SECOND_opcode || op->opcode == GB_PAIR_opcode)
    { 
        // first input is unused, so A is always compatible
        ;
    }
    else if (!GB_Type_compatible (atype, op->xtype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for z=%s(x,y):\n"
            "first input of type [%s]\n"
            "cannot be typecast to x input of type [%s]",
            op->name, atype->name, op->xtype->name))) ;
    }

    //--------------------------------------------------------------------------
    // second input B is cast into the type of op->ytype
    //--------------------------------------------------------------------------

    if (op->opcode == GB_FIRST_opcode || op->opcode == GB_PAIR_opcode)
    { 
        // second input is unused, so B is always compatible
        ;
    }
    else if (btype != NULL)
    {
        // B has a type
        if (!GB_Type_compatible (btype, op->ytype))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "incompatible type for z=%s(x,y):\n"
                "second input of type [%s]\n"
                "cannot be typecast to y input of type [%s]",
                op->name, btype->name, op->ytype->name))) ;
        }
    }
    else
    {
        // B has a just a type code, not a type
        if (!GB_code_compatible (bcode, op->ytype->code))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "incompatible type for z=%s(x,y):\n"
                "second input of type [%s]\n"
                "cannot be typecast to y input of type [%s]",
                op->name, GB_code_string (bcode), op->ytype->name))) ;
        }
    }

    //--------------------------------------------------------------------------
    // result of binary operator of op->ztype is cast to C
    //--------------------------------------------------------------------------

    if (ctype != NULL && !GB_Type_compatible (ctype, op->ztype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for z=%s(x,y):\n"
            "operator output z of type [%s]\n"
            "cannot be typecast to result of type [%s]",
            op->name, op->ztype->name, ctype->name))) ;
    }

    return (GrB_SUCCESS) ;
}


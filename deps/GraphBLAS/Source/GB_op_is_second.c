//------------------------------------------------------------------------------
// GB_op_is_second: return true if op is the SECOND operator of the right type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_op_is_second    // return true if op is SECOND, of the right type
(
    GrB_BinaryOp op,
    GrB_Type type
)
{

    if (op == NULL)
    { 
        // op is NULL, which is interpretted as the implied SECOND operator
        // of the right type
        return (true) ;
    }
    else if (op->opcode == GB_SECOND_opcode)
    {
        // op is the explict SECOND operator; check its type
        if (type == NULL)
        { 
            // type is implicitly the right type
            return (true) ;
        }
        else if (op->ytype == type && op->ztype == type && op->xtype == type)
        { 
            // type is explicitly the right type
            return (true) ;
        }
        else
        { 
            // wrong type
            return (false) ;
        }
    }
    else
    { 
        // wrong opcode
        return (false) ;
    }
}


//------------------------------------------------------------------------------
// GB_BinaryOp_check: check and print a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_BinaryOp_check  // check a GraphBLAS binary operator
(
    const GrB_BinaryOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // print level
    FILE *f                 // file for output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("\n    GraphBLAS BinaryOp: %s ", GB_NAME) ;

    if (op == NULL)
    { 
        // this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (op, "BinaryOp") ;

    GB_Opcode opcode = op->opcode ;
    if (opcode >= GB_USER_opcode)
    { 
        GBPR0 ("(user-defined) ") ;
    }
    else
    { 
        GBPR0 ("(built-in) ") ;
    }

    GBPR0 ("z=%s(x,y)\n", op->name) ;

    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    bool op_is_first  = (opcode == GB_FIRST_opcode) ;
    bool op_is_second = (opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = (opcode == GB_PAIR_opcode) ;

    if (!(op_is_positional || op_is_first || op_is_second)
       && op->function == NULL)
    { 
        GBPR0 ("    BinaryOp has a NULL function pointer\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    if (opcode < GB_FIRST_opcode || opcode > GB_USER_opcode)
    { 
        GBPR0 ("    BinaryOp has an invalid opcode\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    GrB_Info info ;

    info = GB_Type_check (op->ztype, "ztype", pr, f) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("    BinaryOp has an invalid ztype\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    if (!op_is_positional && !op_is_pair)
    {
        if (!op_is_second)
        {
            info = GB_Type_check (op->xtype, "xtype", pr, f) ;
            if (info != GrB_SUCCESS)
            { 
                GBPR0 ("    BinaryOp has an invalid xtype\n") ;
                return (GrB_INVALID_OBJECT) ;
            }
        }

        if (!op_is_first)
        {
            info = GB_Type_check (op->ytype, "ytype", pr, f) ;
            if (info != GrB_SUCCESS)
            { 
                GBPR0 ("    BinaryOp has an invalid ytype\n") ;
                return (GrB_INVALID_OBJECT) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}


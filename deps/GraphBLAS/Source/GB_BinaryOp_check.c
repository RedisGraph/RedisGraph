//------------------------------------------------------------------------------
// GB_BinaryOp_check: check and print a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GB_PUBLIC
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

    GBPR0 ("\n    GraphBLAS BinaryOp: %s ", ((name != NULL) ? name : "")) ;

    if (op == NULL)
    { 
        // this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }
    else if (op == GxB_IGNORE_DUP)
    { 
        // this is a valid dup operator for build
        GBPR0 ("ignore_dup\n") ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (op) ;
    GB_Opcode opcode = op->opcode ;
    if (!GB_IS_BINARYOP_CODE (opcode))
    { 
        GBPR0 ("    BinaryOp has an invalid opcode\n") ;
        return (GrB_INVALID_OBJECT) ;
    }
    if (opcode == GB_USER_binop_code)
    { 
        GBPR0 ("(user-defined) ") ;
    }
    else
    { 
        GBPR0 ("(built-in) ") ;
    }
    GBPR0 ("z=%s(x,y)\n", op->name) ;

    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    bool op_is_first  = (opcode == GB_FIRST_binop_code) ;
    bool op_is_second = (opcode == GB_SECOND_binop_code) ;
    bool op_is_pair   = (opcode == GB_PAIR_binop_code) ;

    if (!(op_is_positional || op_is_first || op_is_second)
       && op->binop_function == NULL)
    { 
        GBPR0 ("    BinaryOp has a NULL function pointer\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    GrB_Info info = GB_Type_check (op->ztype, "ztype", pr, f) ;
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

    if (op->defn != NULL)
    { 
        GBPR0 ("%s\n", op->defn) ;
    }

    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GB_BinaryOp_check: check and print a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// for additional diagnostics, use:
// #define GB_DEVELOPER 1

#include "GB.h"

GrB_Info GB_BinaryOp_check  // check a GraphBLAS binary operator
(
    const GrB_BinaryOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("\nGraphBLAS BinaryOp: %s ", GB_NAME) ;

    if (op == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (op, "BinaryOp") ;

    if (pr > 0)
    {
        if (op->opcode == GB_USER_C_opcode)
        { 
            GBPR ("(compile-time user-defined) ") ;
        }
        else if (op->opcode == GB_USER_R_opcode)
        { 
            GBPR ("(run-time user-defined) ") ;
        }
        else
        { 
            GBPR ("(built-in) ") ;
        }
    }

    GBPR0 ("z=%s(x,y)\n", op->name) ;

    if (op->function == NULL)
    { 
        GBPR0 ("BinaryOp has a NULL function pointer\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "BinaryOp has a NULL function pointer: %s [%s]",
            GB_NAME, op->name))) ;
    }

    if (op->opcode < GB_FIRST_opcode || op->opcode > GB_USER_R_opcode)
    { 
        GBPR0 ("BinaryOp has an invalid opcode\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "BinaryOp has an invalid opcode: %s [%s]", GB_NAME, op->name))) ;
    }

    GrB_Info info ;

    info = GB_Type_check (op->ztype, "ztype", pr, f, Context) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("BinaryOp has an invalid ztype\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "BinaryOp has an invalid ztype: %s [%s]", GB_NAME, op->name))) ;
    }

    info = GB_Type_check (op->xtype, "xtype", pr, f, Context) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("BinaryOp has an invalid xtype\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "BinaryOp has an invalid xtype: %s [%s]", GB_NAME, op->name))) ;
    }

    info = GB_Type_check (op->ytype, "ytype", pr, f, Context) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("BinaryOp has an invalid ytype\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "BinaryOp has an invalid ytype: %s [%s]", GB_NAME, op->name))) ;
    }

    return (GrB_SUCCESS) ;
}


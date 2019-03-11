//------------------------------------------------------------------------------
// GB_UnaryOp_check: check and print a unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// not parallel: this function does O(1) work and is already thread-safe.

#include "GB.h"

GrB_Info GB_UnaryOp_check   // check a GraphBLAS unary operator
(
    const GrB_UnaryOp op,   // GraphBLAS operator to print and check
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

    if (pr > 0) GBPR ("\nGraphBLAS UnaryOp: %s: ", GB_NAME) ;

    if (op == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) GBPR ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (op, "UnaryOp") ;

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

    if (pr > 0) GBPR ("z=%s(x)\n", op->name) ;

    if (op->function == NULL)
    { 
        if (pr > 0) GBPR ("function pointer is NULL\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "UnaryOp has a NULL function pointer: %s [%s]",
            GB_NAME, op->name))) ;
    }

    if (!(op->opcode == GB_ONE_opcode ||
          op->opcode == GB_IDENTITY_opcode ||
          op->opcode == GB_AINV_opcode ||
          op->opcode == GB_ABS_opcode ||
          op->opcode == GB_MINV_opcode ||
          op->opcode == GB_LNOT_opcode ||
          op->opcode == GB_USER_C_opcode ||       // unary or binary
          op->opcode == GB_USER_R_opcode))        // unary or binary
    { 
        if (pr > 0) GBPR ("invalid opcode\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "UnaryOp has an invalid opcode: %s [%s]", GB_NAME, op->name))) ;
    }

    GrB_Info info ;

    info = GB_Type_check (op->ztype, "ztype", pr, f, Context) ;
    if (info != GrB_SUCCESS)
    { 
        if (pr > 0) GBPR ("UnaryOP has an invalid ztype\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "UnaryOp has an invalid ztype: %s [%s]", GB_NAME, op->name))) ;
    }

    info = GB_Type_check (op->xtype, "xtype", pr, f, Context) ;
    if (info != GrB_SUCCESS)
    { 
        if (pr > 0) GBPR ("UnaryOP has an invalid xtype\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "UnaryOp has an invalid xtype: %s [%s]", GB_NAME, op->name))) ;
    }

    return (GrB_SUCCESS) ;
}


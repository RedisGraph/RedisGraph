//------------------------------------------------------------------------------
// GB_BinaryOp_check: check and print a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_BinaryOp_check  // check a GraphBLAS binary operator
(
    const GrB_BinaryOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    const GB_diagnostic pr  // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
)
{


    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (pr > 0) printf ("\nGraphBLAS BinaryOp: %s ", NAME) ;

    if (op == NULL)
    {
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) printf ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    CHECK_MAGIC (op, "BinaryOp") ;

    if (pr > 0 && op->opcode == GB_USER_opcode) printf ("user-defined: ") ;

    if (pr > 0) printf ("z=%s(x,y)\n", op->name) ;

    if (op->function == NULL)
    {
        if (pr > 0) printf ("BinaryOp has a NULL function pointer\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "BinaryOp has a NULL function pointer: %s [%s]", NAME, op->name))) ;
    }

    if (op->opcode < GB_FIRST_opcode || op->opcode > GB_USER_opcode)
    {
        if (pr > 0) printf ("BinaryOp has an invalid opcode\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "BinaryOp has an invalid opcode: %s [%s]", NAME, op->name))) ;
    }

    GrB_Info info ;

    info = GB_Type_check (op->ztype, "ztype", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("BinaryOp has an invalid ztype\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "BinaryOp has an invalid ztype: %s [%s]", NAME, op->name))) ;
    }

    info = GB_Type_check (op->xtype, "xtype", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("BinaryOp has an invalid xtype\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "BinaryOp has an invalid xtype: %s [%s]", NAME, op->name))) ;
    }

    info = GB_Type_check (op->ytype, "ytype", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("BinaryOp has an invalid ytype\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "BinaryOp has an invalid ytype: %s [%s]", NAME, op->name))) ;
    }

    return (GrB_SUCCESS) ; // not REPORT_SUCCESS; may mask error in caller
}


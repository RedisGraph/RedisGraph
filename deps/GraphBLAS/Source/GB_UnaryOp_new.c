//------------------------------------------------------------------------------
// GB_UnaryOp_new: create a new unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// a unary operator: z = f (x).  The unary function signature must be
// void f (void *z, const void *x), and then it must recast its input and
// output arguments internally as needed.

// This function is not directly user-callable.  Use GrB_UnaryOp_new instead.

#include "GB.h"

GrB_Info GB_UnaryOp_new             // create a new user-defined unary operator
(
    GrB_UnaryOp *unaryop,           // handle for the new unary operator
    void *function,                 // pointer to the unary function
    const GrB_Type ztype,           // type of output z
    const GrB_Type xtype,           // type of input x
    const char *name                // name of the function
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_UnaryOp_new (unaryop, function, ztype, xtype)") ;
    RETURN_IF_NULL (unaryop) ;
    (*unaryop) = NULL ;
    RETURN_IF_NULL (function) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (ztype) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (xtype) ;
    ASSERT (name != NULL) ;

    //--------------------------------------------------------------------------
    // create the unary op
    //--------------------------------------------------------------------------

    // allocate the unary operator
    GB_CALLOC_MEMORY (*unaryop, 1, sizeof (GB_UnaryOp_opaque)) ;
    if (*unaryop == NULL)
    {
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG, "out of memory"))) ;
    }

    // initialize the unary operator
    GrB_UnaryOp op = *unaryop ;
    op->magic = MAGIC ;
    op->xtype = xtype ;
    op->ztype = ztype ;
    op->function = function ;
    strncpy (op->name, name, GB_LEN-1) ;
    op->opcode = GB_USER_opcode ;           // generic opcode for all user ops
    ASSERT_OK (GB_check (op, "new user-defined unary op", 0)) ;
    return (REPORT_SUCCESS) ;
}


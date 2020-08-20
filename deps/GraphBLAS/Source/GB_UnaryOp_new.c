//------------------------------------------------------------------------------
// GB_UnaryOp_new: create a new unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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
    GxB_unary_function function,    // pointer to the unary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    const char *name                // name of the function
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_UnaryOp_new (unaryop, function, ztype, xtype)") ;
    GB_RETURN_IF_NULL (unaryop) ;
    (*unaryop) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ztype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (xtype) ;

    //--------------------------------------------------------------------------
    // create the unary op
    //--------------------------------------------------------------------------

    // allocate the unary operator
    GB_CALLOC_MEMORY (*unaryop, 1, sizeof (struct GB_UnaryOp_opaque)) ;
    if (*unaryop == NULL)
    { 
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }

    // initialize the unary operator
    GrB_UnaryOp op = *unaryop ;
    op->magic = GB_MAGIC ;
    op->xtype = xtype ;
    op->ztype = ztype ;
    op->function = function ;
    strncpy (op->name, name, GB_LEN-1) ;
    op->opcode = GB_USER_opcode ;     // user-defined operator
    ASSERT_UNARYOP_OK (op, "new user-defined unary op", GB0) ;
    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GB_BinaryOp_new: create a new binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

// a binary operator: z = f (x,y).  The binary function signature must be
// void f (void *z, const void *x, const void *y), and then it must recast its
// input and output arguments internally as needed.

// This function is not directly user-callable.  Use GrB_BinaryOp_new instead.

GrB_Info GB_BinaryOp_new
(
    GrB_BinaryOp *binaryop,         // handle for the new binary operator
    GxB_binary_function function,   // pointer to the binary function
    const GrB_Type ztype,           // type of output z
    const GrB_Type xtype,           // type of input x
    const GrB_Type ytype,           // type of input y
    const char *name                // name of the function
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_BinaryOp_new (binaryop, function, ztype, xtype, ytype)") ;
    GB_RETURN_IF_NULL (binaryop) ;
    (*binaryop) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ztype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (xtype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ytype) ;

    //--------------------------------------------------------------------------
    // create the binary op
    //--------------------------------------------------------------------------

    // allocate the binary operator
    GB_CALLOC_MEMORY (*binaryop, 1, sizeof (struct GB_BinaryOp_opaque)) ;
    if (*binaryop == NULL)
    { 
        return (GB_NO_MEMORY) ;
    }

    // initialize the binary operator
    GrB_BinaryOp op = *binaryop ;
    op->magic = GB_MAGIC ;
    op->xtype = xtype ;
    op->ytype = ytype ;
    op->ztype = ztype ;
    op->function = function ;
    strncpy (op->name, name, GB_LEN-1) ;
    op->opcode = GB_USER_R_opcode ;     // run-time user-defined operator
    ASSERT_OK (GB_check (op, "new user-defined binary op", GB0)) ;
    return (GrB_SUCCESS) ;
}


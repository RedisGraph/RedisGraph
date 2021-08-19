//------------------------------------------------------------------------------
// GB_UnaryOp_new: create a new unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// a unary operator: z = f (x).  The unary function signature must be
// void f (void *z, const void *x), and then it must recast its input and
// output arguments internally as needed.

// This function is not directly user-callable.  Use GrB_UnaryOp_new instead.

#include "GB.h"
#include <ctype.h>

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

    GB_WHERE1 ("GrB_UnaryOp_new (unaryop, function, ztype, xtype)") ;
    GB_RETURN_IF_NULL (unaryop) ;
    (*unaryop) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ztype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (xtype) ;

    //--------------------------------------------------------------------------
    // create the unary op
    //--------------------------------------------------------------------------

    // allocate the unary operator
    size_t header_size ;
    (*unaryop) = GB_MALLOC (1, struct GB_UnaryOp_opaque, &header_size) ;
    if (*unaryop == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // initialize the unary operator
    GrB_UnaryOp op = *unaryop ;
    op->magic = GB_MAGIC ;
    op->header_size = header_size ;
    op->xtype = xtype ;
    op->ztype = ztype ;
    op->function = function ;
    op->opcode = GB_USER_opcode ;     // user-defined operator
    op->name [0] = '\0' ;

    //--------------------------------------------------------------------------
    // find the name of the operator
    //--------------------------------------------------------------------------

    if (name != NULL)
    {
        // see if the typecast "(GxB_unary_function)" appears in the name
        char *p = NULL ;
        p = strstr ((char *) name, "GxB_unary_function") ;
        if (p != NULL)
        { 
            // skip past the typecast, the left parenthesis, and any whitespace
            p += 19 ;
            while (isspace (*p)) p++ ;
            if (*p == ')') p++ ;
            while (isspace (*p)) p++ ;
            strncpy (op->name, p, GB_LEN-1) ;
        }
        else
        { 
            // copy the entire name as-is
            strncpy (op->name, name, GB_LEN-1) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_UNARYOP_OK (op, "new user-defined unary op", GB0) ;
    return (GrB_SUCCESS) ;
}


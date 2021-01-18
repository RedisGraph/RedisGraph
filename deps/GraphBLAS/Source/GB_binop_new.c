//------------------------------------------------------------------------------
// GB_binop_new: create a new operator (user-defined or internal)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Create a new a binary operator: z = f (x,y).  The function pointer may
// be NULL, for implied functions (FIRST and SECOND).  It may not be NULL
// otherwise.

#include "GB.h"
#include "GB_binop.h"
#include <ctype.h>

GrB_Info GB_binop_new
(
    GrB_BinaryOp *binaryop,         // handle for the new binary operator
    GxB_binary_function function,   // binary function (may be NULL)
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype,                 // type of input y
    const char *name,               // name of the function (may be NULL)
    const GB_Opcode opcode          // opcode for the function
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (binaryop != NULL) ;
    ASSERT (ztype != NULL) ;
    ASSERT (xtype != NULL) ;
    ASSERT (ytype != NULL) ;

    //--------------------------------------------------------------------------
    // create the binary op
    //--------------------------------------------------------------------------

    // allocate the binary operator
    (*binaryop) = GB_CALLOC (1, struct GB_BinaryOp_opaque) ;
    if (*binaryop == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // initialize the binary operator
    GrB_BinaryOp op = *binaryop ;
    op->magic = GB_MAGIC ;
    op->xtype = xtype ;
    op->ytype = ytype ;
    op->ztype = ztype ;
    op->function = function ;       // may be NULL
    op->opcode = opcode ;

    //--------------------------------------------------------------------------
    // find the name of the operator
    //--------------------------------------------------------------------------

    if (name == NULL)
    { 
        // if no name , a generic name is used instead
        strncpy (op->name, "user_binary_operator", GB_LEN-1) ;
    }
    else
    {
        // see if the typecast "(GxB_binary_function)" appears in the name
        char *p = NULL ;
        p = strstr ((char *) name, "GxB_binary_function") ;
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

    ASSERT_BINARYOP_OK (op, "new binary op", GB0) ;
    return (GrB_SUCCESS) ;
}


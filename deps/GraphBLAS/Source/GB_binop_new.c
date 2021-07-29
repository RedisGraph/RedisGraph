//------------------------------------------------------------------------------
// GB_binop_new: create a new operator (user-defined or internal)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Create a new a binary operator: z = f (x,y).  The function pointer may
// be NULL, for implied functions (FIRST and SECOND).  It may not be NULL
// otherwise.

// The binary op header is allocated by the caller, and passed in
// uninitialized.

#include "GB.h"
#include "GB_binop.h"
#include <ctype.h>

void GB_binop_new
(
    GrB_BinaryOp op,                // new binary operator
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

    ASSERT (op != NULL) ;
    ASSERT (ztype != NULL) ;
    ASSERT (xtype != NULL) ;
    ASSERT (ytype != NULL) ;

    //--------------------------------------------------------------------------
    // initialize the binary operator
    //--------------------------------------------------------------------------

    op->magic = GB_MAGIC ;
    op->xtype = xtype ;
    op->ytype = ytype ;
    op->ztype = ztype ;
    op->function = function ;       // may be NULL
    op->opcode = opcode ;
    op->name [0] = '\0' ;

    //--------------------------------------------------------------------------
    // find the name of the operator
    //--------------------------------------------------------------------------

    if (name != NULL)
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
}


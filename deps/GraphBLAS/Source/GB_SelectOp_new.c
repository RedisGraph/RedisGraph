//------------------------------------------------------------------------------
// GB_SelectOp_new: create a new select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The select function signature must be:

//      bool f (GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols,
//              const void *x, const void *thunk) ;

#include "GB.h"
#include <ctype.h>

GrB_Info GB_SelectOp_new        // create a new user-defined select operator
(
    GxB_SelectOp *selectop,     // handle for the new select operator
    GxB_select_function function,// pointer to the select function
    GrB_Type xtype,             // type of input x
    GrB_Type ttype,             // type of input thunk
    const char *name            // name of the function
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_SelectOp_new (selectop, function, xtype)") ;
    GB_RETURN_IF_NULL (selectop) ;
    (*selectop) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_FAULTY (xtype) ;   // xtype may be NULL
    GB_RETURN_IF_FAULTY (ttype) ;   // ttype may be NULL

    //--------------------------------------------------------------------------
    // create the select op
    //--------------------------------------------------------------------------

    // allocate the select operator
    size_t header_size ;
    (*selectop) = GB_MALLOC (1, struct GB_SelectOp_opaque, &header_size) ;
    if (*selectop == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // initialize the select operator
    GxB_SelectOp op = *selectop ;
    op->magic = GB_MAGIC ;
    op->header_size = header_size ;
    op->xtype = xtype ;
    op->ttype = ttype ;
    op->function = function ;
    op->opcode = GB_USER_SELECT_opcode ;
    op->name [0] = '\0' ;

    //--------------------------------------------------------------------------
    // find the name of the operator
    //--------------------------------------------------------------------------

    if (name != NULL)
    {
        // see if the typecast "(GxB_select_function)" appears in the name
        char *p = NULL ;
        p = strstr ((char *) name, "GxB_select_function") ;
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

    ASSERT_SELECTOP_OK (op, "new user-defined select op", GB0) ;
    return (GrB_SUCCESS) ;
}


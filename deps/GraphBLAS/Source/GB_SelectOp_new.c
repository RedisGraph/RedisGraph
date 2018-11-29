//------------------------------------------------------------------------------
// GB_SelectOp_new: create a new select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The select function signature must be:

//      bool f (GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols,
//              const void *x, const void *k) ;

// This function is not directly user-callable.  Use GxB_SelectOp_new instead.

#include "GB.h"

GrB_Info GB_SelectOp_new        // create a new user-defined select operator
(
    GxB_SelectOp *selectop,     // handle for the new select operator
    GxB_select_function function,// pointer to the select function
    const GrB_Type xtype,       // type of input x
    const char *name            // name of the function
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_SelectOp_new (selectop, function, xtype)") ;
    GB_RETURN_IF_NULL (selectop) ;
    (*selectop) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_FAULTY (xtype) ;   // xtype may be NULL

    //--------------------------------------------------------------------------
    // create the select op
    //--------------------------------------------------------------------------

    // allocate the select operator
    GB_CALLOC_MEMORY (*selectop, 1, sizeof (struct GB_SelectOp_opaque)) ;
    if (*selectop == NULL)
    { 
        return (GB_NO_MEMORY) ;
    }

    // initialize the select operator
    GxB_SelectOp op = *selectop ;
    op->magic = GB_MAGIC ;
    op->xtype = xtype ;
    op->function = function ;
    strncpy (op->name, name, GB_LEN-1) ;
    op->opcode = GB_USER_SELECT_R_opcode ;
    ASSERT_OK (GB_check (op, "new user-defined select op", GB0)) ;
    return (GrB_SUCCESS) ;
}


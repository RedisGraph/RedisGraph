//------------------------------------------------------------------------------
// GxB_SelectOp_new: create a new user-defined select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GxB_SelectOp_new is implemented both as a macro and a function.  Both are
// user-callable.  The macro is used by default since it can capture the name
// of the select function.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GxB_SelectOp_new

GrB_Info GxB_SelectOp_new       // create a new user-defined select operator
(
    GxB_SelectOp *selectop,     // handle for the new select operator
    GxB_select_function function,// pointer to the select function
    GrB_Type xtype,             // type of input x, or NULL if type-generic
    GrB_Type ttype              // type of input thunk, or NULL if not used
)
{ 
    return (GB_SelectOp_new (selectop, function, xtype, ttype, "f")) ;
}


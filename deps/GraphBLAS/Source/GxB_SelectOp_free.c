//------------------------------------------------------------------------------
// GxB_SelectOp_free: free a select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_SelectOp_free          // free a user-created select operator
(
    GxB_SelectOp *op                // handle of select operator to free
)
{ 
    return (GB_Op_free ((GB_Operator *) op)) ;
}


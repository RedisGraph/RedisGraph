//------------------------------------------------------------------------------
// GrB_IndexUnaryOp_free: free an index_unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_IndexUnaryOp_free          // free a user-created index_unary op
(
    GrB_IndexUnaryOp *op                // handle of operator to free
)
{ 
    return (GB_Op_free ((GB_Operator *) op)) ;
}


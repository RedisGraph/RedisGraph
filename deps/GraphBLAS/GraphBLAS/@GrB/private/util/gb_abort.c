//------------------------------------------------------------------------------
// gb_abort: terminate a GraphBLAS function
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "gb_matlab.h"

void gb_abort ( void )      // failure
{
    mexErrMsgIdAndTxt ("GraphBLAS:abort", "GraphBLAS failed") ;
}


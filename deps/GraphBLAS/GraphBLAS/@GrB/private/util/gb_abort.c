//------------------------------------------------------------------------------
// gb_abort: terminate a GraphBLAS function
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

#include "gb_interface.h"

void gb_abort ( void )      // failure
{
    mexErrMsgIdAndTxt ("GraphBLAS:abort", "GraphBLAS failed") ;
}


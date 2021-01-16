//------------------------------------------------------------------------------
// GB_mx_clear_time: clear the time and start the timer
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_clear_time ( )               // clear the time and start the tic
{

    grbtime = 0 ;
    tic [0] = 0 ;
    tic [1] = 0 ;

    simple_tic (tic) ;
}


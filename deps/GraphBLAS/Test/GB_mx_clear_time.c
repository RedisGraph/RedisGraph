//------------------------------------------------------------------------------
// GB_mx_clear_time: clear the time and start the timer
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_clear_time ( )               // clear the time and start the tic
{

    grbtime = 0 ;
    tic [0] = 0 ;
    tic [1] = 0 ;

    simple_tic (tic) ;
}


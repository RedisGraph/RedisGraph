//------------------------------------------------------------------------------
// gb_flush: flush mexPrintf output to MATLAB Command Window
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// When GraphBLAS is under development, it is essential that output from
// mexPrintf be flushed to the MATLAB Command Window as soon as possible.  This
// function ensures that this is done.  The function is also used in
// production, when a MATLAB function wishes to display the contents of a GrB
// object, via disp or display.  This ensures that all output from GraphBLAS is
// immediately visible in the MATLAB Command Window.

// https://www.mathworks.com/matlabcentral/answers/255615-why-are-mex-file-output-messages-using-mexprintf-mexevalstring-drawnow-not-printed-immediatel

// gb_flush ( ) returns 0 if successful, or any nonzero value on failure.  This
// matches the behavior of the ANSI C fflush.

#include "gb_matlab.h"

int gb_flush ( void )       // flush mexPrintf output to MATLAB Command Window
{
    // 'drawnow' is slow when logging in remotely: disable it.
    // return (mexEvalString ("drawnow ; pause (1e-8) ;")) ;
    return (mexEvalString ("pause (1e-8) ;")) ;
}


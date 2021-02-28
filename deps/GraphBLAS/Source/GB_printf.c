//------------------------------------------------------------------------------
// GB_printf.c: printing for GraphBLAS *check functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GxB_*_fprintf with f == NULL prints nothing by default.  However, if
// GB_printf_function has been set by the caller, then that function is used
// when f is NULL.

#include "GB_printf.h"

int (* GB_printf_function ) (const char *format, ...) = NULL ;


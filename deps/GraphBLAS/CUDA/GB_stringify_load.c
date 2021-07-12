//------------------------------------------------------------------------------
// GB_stringify_load: return a string to load/save a value
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// TODO: the typecast should be handled better.  See ../Source/GB_casting.h,
// and note the inline functions to cast from double to integer.

// Construct a macro to load and typecast.  For example:
//  
//  #define GB_GETA(blob) blob
//
// then use as:
//      GB_GETA (double aij = Ax [p]) ;
//      GB_GETA (double *Ax = A->x) ;
//      GB_GETA (T_A *restrict Ax = A->x) ;
//
// which become
//      double aij = Ax [p] ;
//      double *Ax = A->x ;
//      T_A *Ax = A->x ;
//
// or, if is_pattern is true, the macro becomes the empty string.

#include "GB.h"
#include "GB_stringify.h"

void GB_stringify_load         // return a string to load/typecast macro
(
    // output:
    char *load_macro,               // string with #define macro to load value
    // input:
    const char *load_macro_name,    // name of macro to construct
    bool is_pattern                 // if true, load/cast does nothing
)
{

    if (is_pattern)
    {
        snprintf (load_macro, GB_CUDA_STRLEN, "#define %s(blob)",
            load_macro_name) ;
    }
    else
    {
        snprintf (load_macro, GB_CUDA_STRLEN, "#define %s(blob) blob",
            load_macro_name) ;
    }
}


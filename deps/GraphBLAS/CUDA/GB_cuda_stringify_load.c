// SPDX-License-Identifier: Apache-2.0

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
#include "GB_cuda_stringify.h"

void GB_cuda_stringify_load    // return a string to load/typecast macro
(
    // output:
    char *result,
    // input:
    const char *macro_name,       // name of macro to construct
    bool is_pattern         // if true, load/cast does nothing
)
{

    if (is_pattern)
    {
        snprintf (result, GB_CUDA_STRLEN, "#define %s(blob)", macro_name) ;
    }
    else
    {
        snprintf (result, GB_CUDA_STRLEN, "#define %s(blob) blob", macro_name) ;
    }
}


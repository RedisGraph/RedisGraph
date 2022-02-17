//------------------------------------------------------------------------------
// GB_size_t_multiply:  multiply two size_t and guard against overflow
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// c = a*b but check for overflow

#include "GB.h"

GB_PUBLIC
bool GB_size_t_multiply     // true if ok, false if overflow
(
    size_t *c,              // c = a*b, or zero if overflow occurs
    const size_t a,
    const size_t b
)
{

    ASSERT (c != NULL) ;

    (*c) = 0 ;
    if (a == 0 || b == 0)
    { 
        return (true) ;
    }

    if (a > SIZE_MAX / 2 || b > SIZE_MAX / 2)
    { 
        // a or b are out of range
        return (false) ;
    }

    // a + b is now safe to compute
    if ((a + b) > (SIZE_MAX / GB_IMIN (a,b)))
    { 
        // a * b may overflow
        return (false) ;
    }

    // a * b will not overflow
    (*c) = a * b ;
    return (true) ;
}


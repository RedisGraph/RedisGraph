//------------------------------------------------------------------------------
// GB_Index_multiply:  multiply two integers and guard against overflow
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// c = a*b where c is GrB_Index (uint64_t), and a and b are int64_t.
// Check for overflow.  Requires a >= 0 and b >= 0.

#include "GB.h"

bool GB_Index_multiply      // true if ok, false if overflow
(
    GrB_Index *restrict c,  // c = a*b, or zero if overflow occurs
    const int64_t a,
    const int64_t b
)
{

    ASSERT (c != NULL) ;

    (*c) = 0 ;
    if (a == 0 || b == 0)
    { 
        return (true) ;
    }

    if (a < 0 || a > GxB_INDEX_MAX || b < 0 || b > GxB_INDEX_MAX)
    { 
        // a or b are out of range
        return (false) ;
    }

    if (GB_CEIL_LOG2 (a) + GB_CEIL_LOG2 (b) > 60)
    { 
        // a * b may overflow
        return (false) ;
    }

    // a * b will not overflow
    (*c) = a * b ;
    return (true) ;
}


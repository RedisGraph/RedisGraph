//------------------------------------------------------------------------------
// GB_bracket.h: definitions for GB_bracket
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_BRACKET_H
#define GB_BRACKET_H
#include "GB.h"

//------------------------------------------------------------------------------
// GB_bracket_left
//------------------------------------------------------------------------------

// Given a sorted list X [kleft:kright], and a range imin:..., modify kleft so
// that the smaller sublist X [kleft:kright] contains the range imin:...

#if 0

// no longer used

static inline void GB_bracket_left
(
    const int64_t imin,
    const int64_t *GB_RESTRICT X,  // input list is in X [kleft:kright]
    int64_t *kleft,
    const int64_t kright
)
{
    ASSERT (DEAD_CODE) ;
    // tighten kleft
    int64_t len = kright - (*kleft) + 1 ;
    if (len > 0 && X [(*kleft)] < imin)
    { 
        // search for imin in X [kleft:kright]
        int64_t pleft = (*kleft) ;
        int64_t pright = kright ;
        GB_TRIM_BINARY_SEARCH (imin, X, pleft, pright) ;
        (*kleft) = pleft ;
    }
}

#endif

//------------------------------------------------------------------------------
// GB_bracket_right
//------------------------------------------------------------------------------

// Given a sorted list X [kleft:kright], and a range ...:imax, modify kright so
// that the smaller sublist X [kleft:kright] contains the range ...:imax.

static inline void GB_bracket_right
(
    const int64_t imax,
    const int64_t *GB_RESTRICT X,  // input list is in X [kleft:kright]
    const int64_t kleft,
    int64_t *kright
)
{
    // tighten kright
    int64_t len = (*kright) - kleft + 1 ;
    if (len > 0 && imax < X [(*kright)])
    { 
        // search for imax in X [kleft:kright]
        int64_t pleft = kleft ;
        int64_t pright = (*kright) ;
        GB_TRIM_BINARY_SEARCH (imax, X, pleft, pright) ;
        (*kright) = pleft ;
    }
}

//------------------------------------------------------------------------------
// GB_bracket
//------------------------------------------------------------------------------

// Given a sorted list X [kleft:kright], and a range imin:imax, find the
// kleft_new and kright_new so that the smaller sublist X
// [kleft_new:kright_new] contains the range imin:imax.

// Zombies are not tolerated.

#if 0

static inline void GB_bracket
(
    const int64_t imin,         // search for entries in the range imin:imax
    const int64_t imax,
    const int64_t *GB_RESTRICT X,  // input list is in X [kleft:kright]
    const int64_t kleft_in,
    const int64_t kright_in,
    int64_t *kleft_new,         // output list is in X [kleft_new:kright_new]
    int64_t *kright_new
)
{ 
    ASSERT (DEAD_CODE) ;

    int64_t kleft  = kleft_in ;
    int64_t kright = kright_in ;

    if (imin > imax)
    { 
        // imin:imax is empty, make X [kleft:kright] empty
        (*kleft_new ) = kleft ;
        (*kright_new) = kleft-1 ;
        return ;
    }

    // Find kleft and kright so that X [kleft:kright] contains all of imin:imax

    // tighten kleft
    GB_bracket_left (imin, X, &kleft, kright) ;

    // tighten kright
    GB_bracket_right (imax, X, kleft, &kright) ;

    // list has been trimmed
    ASSERT (GB_IMPLIES (kleft > kleft_in, X [kleft-1] < imin)) ;
    ASSERT (GB_IMPLIES (kright < kright_in, imax < X [kright+1])) ;

    // return result
    (*kleft_new ) = kleft ;
    (*kright_new) = kright ;
}
#endif

#endif
